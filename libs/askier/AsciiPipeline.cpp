#include "askier/AsciiPipeline.hpp"
#include "askier/AsciiMapper.hpp"
#include <opencv2/imgproc.hpp>

#include "askier/AsciiRenderer.hpp"
#include "askier/ImageUtils.hpp"


/**
 * Applies an ordered dithering effect to the given matrix of grayscale cell values
 * using a 4x4 Bayer matrix. The resulting matrix values will be adjusted to add a
 * dithering effect while preserving their range within [0,1].
 * https://en.wikipedia.org/wiki/Ordered_dithering
 * @param cells A cv::Mat of type CV_32FC1 (single-channel 32-bit floating point)
 *              containing grayscale values normalized to the range [0,1].
 *
 */
static void applyOrderedDither(cv::Mat &cells) {
    static const float bayer4x4[4][4] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5}
    };
    // Normalize to [0,1): t = b/16, bias around zero by subtracting 0.5, scale strength.
    constexpr float invN = 1.0f / 16.0f;
    constexpr float strength = 1.0f / 16.0f; // smaller = subtler pattern

    for (int r = 0; r < cells.rows; ++r) {
        float *row = cells.ptr<float>(r);
        for (int c = 0; c < cells.cols; ++c) {
            const float t = bayer4x4[r & 3][c & 3] * invN; // [0,1)
            float v = row[c] + (t - 0.5f) * strength; // bias around 0
            row[c] = std::clamp(v, 0.0f, 1.0f);
        }
    }
}


/**
 * Apply Floyd-Steinberg dithering to the input matrix of grayscale values,
 * diffusing quantization errors to neighboring pixels to improve the visual
 * representation of quantized levels.
 *
 * The dithering process works in-place on the input matrix. It adjusts the
 * pixel values to the nearest quantization level and spreads the quantization
 * error to neighboring pixels using standard Floyd-Steinberg error diffusion
 * weights.
 * https://en.wikipedia.org/wiki/Floyd%E2%80%93Steinberg_dithering
 * @param cells A cv::Mat representing the grayscale image; pixel values must
 *              be in the range [0.0, 1.0]. The matrix is modified in-place.
 * @param levels The number of quantization levels, clamped to the range [2, 256].
 *               Defaults to 32.
 */
static void applyFloydSteinberg(cv::Mat &cells, int levels = 32) {
    levels = std::max(2, std::min(256, levels));
    const float scale = static_cast<float>(levels - 1);
    // Work in-place, processing left-to-right, top-to-bottom.
    for (int y = 0; y < cells.rows; ++y) {
        float *row = cells.ptr<float>(y);
        float *rowDown = (y + 1 < cells.rows) ? cells.ptr<float>(y + 1) : nullptr;
        for (int x = 0; x < cells.cols; ++x) {
            float oldV = std::clamp(row[x], 0.0f, 1.0f);
            float q = std::round(oldV * scale) / scale; // quantized luminance
            row[x] = q;
            float err = oldV - q;
            // Distribute error to neighbors (classic FS weights)
            if (x + 1 < cells.cols) row[x + 1] += err * (7.0f / 16.0f);
            if (rowDown) {
                if (x > 0) rowDown[x - 1] += err * (3.0f / 16.0f);
                rowDown[x] += err * (5.0f / 16.0f);
                if (x + 1 < cells.cols) rowDown[x + 1] += err * (1.0f / 16.0f);
            }
        }
    }
    // Clamp once after diffusion to keep values in range
    for (int y = 0; y < cells.rows; ++y) {
        float *row = cells.ptr<float>(y);
        for (int x = 0; x < cells.cols; ++x) {
            row[x] = std::clamp(row[x], 0.0f, 1.0f);
        }
    }
}


AsciiPipeline::AsciiPipeline(const std::shared_ptr<GlyphDensityCalibrator> &calibrator) : calibrator(calibrator) {
}


AsciiPipeline::Result AsciiPipeline::process(const cv::Mat &bgr, const AsciiParams &params) const {
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

    cv::Mat grayUint;
    cv::cvtColor(bgr, grayUint, cv::COLOR_BGR2GRAY);
    cv::Mat gray;
    grayUint.convertTo(gray, CV_32F);
    gray /= 255.0;
    
    // Apply Sobel edge detection to highlight edges
    cv::Mat sobelX, sobelY, sobel;
    cv::Sobel(gray, sobelX, CV_32F, 2, 0, 5);  // X gradient
    cv::Sobel(gray, sobelY, CV_32F, 0, 2, 5);  // Y gradient
    cv::magnitude(sobelX, sobelY, sobel);       // Combine gradients
    
    // Normalize Sobel result to [0, 1] range
    cv::Mat sobelNorm;
    cv::normalize(sobel, sobelNorm, 0.0, 1.0, cv::NORM_MINMAX);
    
    // Multiply original grayscale with normalized Sobel to highlight edges
    cv::multiply(gray, 1 - sobelNorm, gray);
    
    cv::Mat cells;
    cv::resize(gray, cells, cv::Size(columns, rows), 0, 0, cv::INTER_AREA);

    if (params.dithering == DitheringType::FloydSteinberg) {
        applyFloydSteinberg(cells, 32);
    } else if (params.dithering == DitheringType::Ordered) {
        applyOrderedDither(cells);
    }

    const AsciiMapper mapper(calibrator);
    Result result;
    result.lines.resize(rows);

    for (int row = 0; row < rows; ++row) {
        QString line;
        line.reserve(columns);
        const float *row_ptr = cells.ptr<float>(row);
        for (int col = 0; col < columns; ++col) {
            const auto luminance = static_cast<double>(row_ptr[col]);
            line.push_back(mapper.map(luminance));
        }
        result.lines[row] = std::move(line);
    }
    const AsciiRenderer renderer(calibrator->font());
    result.preview = renderer.render(result.lines);
    cv::Mat midImage;
    cells = cells * 255;
    cells.convertTo(midImage, CV_8UC1);
    result.midImage = matToQImageGray(midImage);
    return result;
}
