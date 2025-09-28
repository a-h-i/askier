#include "askier/AsciimapOCL.hpp"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/ocl.hpp>

#include "askier/GlyphDensityCalibrator.hpp"
#include <iostream>


static std::string kernel_source = R"SRC(
kernel void ascii_map_lut(
__global const uchar *src,
int src_step,
int rows,
int cols,
__global const uchar *lut,
int lut_size,
__global uchar *dst,
int dst_step
)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    if (x >= cols || y >= rows) return;
    const int source_idx = y * src_step + x;
    const int dst_idx = y * dst_step  + x;
    const uchar luminance = src[source_idx];
    const uchar darkness = 255 - luminance;

    const float darkness_ratio = (float) darkness / 255.0f;
    const int max_lut_index = lut_size - 1;
    int darkness_index = (int) round(darkness_ratio * max_lut_index);

    if (darkness_index < 0) {
        darkness_index = 0;
    } else if (darkness_index > max_lut_index) {
       darkness_index = max_lut_index;
    }

    dst[dst_idx] = lut[darkness_index];



}
)SRC";

cv::Mat asciiMapOCL(const cv::UMat &src, const std::remove_reference_t<std::remove_const_t<lut_type> > &lut) {
    CV_Assert(src.type() == CV_8UC1);
    CV_Assert(lut.size() == ASCII_COUNT);
    cv::UMat dst(src.size(), CV_8UC1);

    cv::Mat hostLut(1, static_cast<int>(lut.size()), CV_8UC1);
    for (size_t i = 0; i < lut.size(); i++) {
        hostLut.at<uchar>(0, static_cast<int>(i), 0) = lut[i];
    }
    cv::UMat deviceLut = hostLut.getUMat(cv::ACCESS_READ);


    cv::ocl::ProgramSource source(kernel_source);
    std::string compileErrors;
    cv::ocl::Program program = cv::ocl::Context::getDefault().getProg(source, "", compileErrors);
    if (program.empty()) {
        std::cerr << "Issue with program compilation\n"
                << compileErrors << std::endl;
    }
    cv::ocl::Kernel kernel("ascii_map_lut", program);
    CV_Assert(!kernel.empty());

    CV_Assert(src.isContinuous());
    CV_Assert(hostLut.isContinuous());
    CV_Assert(deviceLut.isContinuous());
    CV_Assert(dst.isContinuous());
    CV_Assert(!hostLut.empty());
    int argIndex = 0;
    kernel.set(argIndex++, cv::ocl::KernelArg::ReadOnlyNoSize(src));
    kernel.set(argIndex++, static_cast<int>(src.step));
    kernel.set(argIndex++, src.rows);
    kernel.set(argIndex++, src.cols);
    kernel.set(argIndex++, cv::ocl::KernelArg::PtrReadOnly(deviceLut));
    kernel.set(argIndex++, static_cast<int>(lut.size()));
    kernel.set(argIndex++, cv::ocl::KernelArg::WriteOnlyNoSize(dst));
    kernel.set(argIndex++, static_cast<int>(dst.step));


    size_t globals[2] = {static_cast<size_t>(src.cols), static_cast<size_t>(src.rows)};

    bool run_ok = kernel.run(2, globals, nullptr, true);
    CV_Assert(run_ok);

    return dst.getMat(cv::ACCESS_READ).clone();
}
