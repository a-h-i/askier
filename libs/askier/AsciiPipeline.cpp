#include "askier/AsciiPipeline.hpp"

#include <iostream>
#include <ranges>

#include "askier/AsciiMapper.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/core/ocl.hpp>
#include <pstl/glue_execution_defs.h>

#include "askier/AsciimapOCL.hpp"
#include "askier/AsciiRenderer.hpp"
#include "askier/ImageUtils.hpp"
#include "askier/Dithering.hpp"

AsciiPipeline::AsciiPipeline(const std::shared_ptr<GlyphDensityCalibrator> &calibrator) : calibrator(calibrator) {
    std::cout << "Using OpenCL: " << cv::ocl::haveOpenCL() << std::endl;
    std::cout << "Number devices: " << cv::ocl::Context::getDefault().ndevices() << std::endl;
    auto device  = cv::ocl::Context::getDefault().device(0);
    clContext = cv::ocl::Context::fromDevice(device);
    std::cout << "Using device: " << device.name() << std::endl;
}


AsciiPipeline::Result AsciiPipeline::process(const cv::Mat &bgr, const AsciiParams &params) {
    if (bgr.empty()) {
        return Result();
    }
    // compute rows from columns and font aspect
    const double aspect = calibrator->cellAspect();
    const int width = bgr.cols;
    const int height = bgr.rows;

    const int columns = std::max(8, params.columns);
    const int rows = std::max(
        4, static_cast<int>(std::round(
            static_cast<double>(height) / static_cast<double>(width) * columns / aspect)));
    cv::UMat bgrGPU = bgr.getUMat(cv::ACCESS_RW);
    cv::UMat grayUint(bgrGPU.size(), CV_8UC1, cv::USAGE_ALLOCATE_DEVICE_MEMORY);
    cv::cvtColor(bgrGPU, grayUint, cv::COLOR_BGR2GRAY);
    cv::UMat gray(grayUint.size(), CV_32F, cv::USAGE_ALLOCATE_DEVICE_MEMORY);
    grayUint.convertTo(gray, CV_32F, 1 / 255.0);

    // Apply Sobel edge detection to highlight edges
    cv::UMat sobelX, sobelY, sobel;
    cv::Sobel(gray, sobelX, CV_32F, 2, 0, 5); // X gradient
    cv::Sobel(gray, sobelY, CV_32F, 0, 2, 5); // Y gradient
    cv::magnitude(sobelX, sobelY, sobel); // Combine gradients
    //
    // Normalize Sobel result to [0, 1] range
    cv::UMat sobelNorm;
    cv::normalize(sobel, sobelNorm, 0.0, 1.0, cv::NORM_MINMAX);
    cv::multiply(sobelNorm, -1, sobelNorm);
    cv::add(sobelNorm, 1, sobelNorm);

    // Multiply original grayscale with normalized Sobel to highlight edges
    cv::multiply(gray, sobelNorm, gray);
    const auto outputSize = cv::Size(columns, rows);
    cv::UMat cells(outputSize, CV_32F, cv::USAGE_ALLOCATE_DEVICE_MEMORY);
    cv::resize(gray, cells, outputSize, 0, 0, cv::INTER_AREA);


    if (params.dithering == DitheringType::FloydSteinberg) {
        applyFloydSteinberg(clContext, cells, 32);
    } else if (params.dithering == DitheringType::Ordered) {
        applyOrderedDither(cells);
    }

    Result result;
    result.lines.resize(rows);


    auto mappedMatrix = ascii_mapper_ocl(clContext, cells, calibrator->lut());

    oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, mappedMatrix.rows),
                              [&mappedMatrix, &result](const oneapi::tbb::blocked_range<int> &range) {
                                  for (int row = range.begin(); row < range.end(); ++row) {
                                      QString line;
                                      line.reserve(mappedMatrix.cols);
                                      const uchar *row_ptr = mappedMatrix.ptr<uchar>(row);
                                      for (int col = 0; col < mappedMatrix.cols; ++col) {
                                          line.push_back(QChar::fromLatin1(row_ptr[col]));
                                      }
                                      result.lines[row] = std::move(line);
                                  }
                              });

    const AsciiRenderer renderer(calibrator->font());
    result.preview = renderer.render(result.lines);
    cv::Mat midImage;
    cells.convertTo(midImage, CV_8UC1, 255);
    result.midImage = matToQImageGray(midImage);
    return result;
}
