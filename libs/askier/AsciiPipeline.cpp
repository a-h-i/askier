#include "askier/AsciiPipeline.hpp"
#include "askier/AsciiMapper.hpp"
#include <opencv2/imgproc.hpp>

#include "askier/AsciiRenderer.hpp"
#include "askier/ImageUtils.hpp"


/**
 * Build (once) a 256-entry inverse sRGB lookup table that maps 8-bit sRGB
 * code values [0..255] -> linear-light floats in [0..1].
 *
 * Using cv::LUT with this table is significantly faster than calling std::pow
 * per pixel and guarantees consistent gamma removal across channels.
 * It uses the standard sRGB formula to
 * handle values below and above a certain threshold (0.04045).
 */
// static const cv::Mat &sRGBInverseLUT256() {
//     static cv::Mat lut;
//     static std::once_flag flag;
//     std::call_once(flag, []() {
//         lut.create(1, 256, CV_32FC1);
//         float *p = lut.ptr<float>();
//         for (int i = 0; i < 256; ++i) {
//             const double c = i / 255.0;
//             const double L = (c <= 0.04045)
//                                  ? (c / 12.92)
//                                  : std::pow((c + 0.055) / 1.055, 2.4);
//             p[i] = static_cast<float>(L);
//         }
//     });

//     return lut;
// }


// static std::vector<cv::Mat> srgbToLinear(const cv::Mat &srgb) {
//     const cv::Mat &lut = sRGBInverseLUT256();
//     std::vector<cv::Mat> channels;
//     cv::split(srgb, channels);
//     std::vector<cv::Mat> linear(channels.size());
//     for (std::size_t i = 0; i < channels.size(); ++i) {
//         cv::LUT(channels[i], lut, linear[i]);
//     }
//     return linear;
// }


/**
 * Compute per-cell average luminance using an integral image.
 * This yields exact means over disjoint pixel bins that partition the source.
 * cells will be of size (rows x cols), type CV_32FC1, with values in [0,1].
 */
static void computeCellMeans(const cv::Mat &grayLinear, int cols, int rows, cv::Mat &cellsOut) {
    const int width = grayLinear.cols;
    const int height = grayLinear.rows;

    // Integral image with 1-pixel padding; double precision for accuracy.
    cv::Mat integralSum;
    cv::integral(grayLinear, integralSum, CV_64F);

    cellsOut.create(rows, cols, CV_32FC1);
    for (int r = 0; r < rows; ++r) {
        // Integer bin edges partitioning the source image
        const int y0 = (r * height) / rows;
        const int y1 = ((r + 1) * height) / rows;
        auto *outRow = cellsOut.ptr<float>(r);
        for (int c = 0; c < cols; ++c) {
            const int x0 = (c * width) / cols;
            const int x1 = ((c + 1) * width) / cols;

            const int area = std::max(1, (x1 - x0) * (y1 - y0));
            const double s =
                    integralSum.at<double>(y1, x1) - integralSum.at<double>(y1, x0) -
                    integralSum.at<double>(y0, x1) + integralSum.at<double>(y0, x0);
            outRow[c] = static_cast<float>(s / static_cast<double>(area));
        }
    }
}

/**
 * Applies an ordered dithering effect to the given matrix of grayscale cell values
 * using a 4x4 Bayer matrix. The resulting matrix values will be adjusted to add a
 * dithering effect while preserving their range within [0,1].
 * https://en.wikipedia.org/wiki/Ordered_dithering
 * @param cells A cv::Mat of type CV_32FC1 (single-channel 32-bit floating point)
 *              containing grayscale values normalized to the range [0,1].
 *
 */
// static void applyOrderedDither(cv::Mat &cells) {
//     static const float bayer4x4[4][4] = {
//         {0, 8, 2, 10},
//         {12, 4, 14, 6},
//         {3, 11, 1, 9},
//         {15, 7, 13, 5}
//     };
//     // Normalize to [0,1): t = b/16, bias around zero by subtracting 0.5, scale strength.
//     constexpr float invN = 1.0f / 16.0f;
//     constexpr float strength = 1.0f / 16.0f; // smaller = subtler pattern

//     for (int r = 0; r < cells.rows; ++r) {
//         float *row = cells.ptr<float>(r);
//         for (int c = 0; c < cells.cols; ++c) {
//             const float t = bayer4x4[r & 3][c & 3] * invN; // [0,1)
//             float v = row[c] + (t - 0.5f) * strength; // bias around 0
//             row[c] = std::clamp(v, 0.0f, 1.0f);
//         }
//     }
// }


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
// static void applyFloydSteinberg(cv::Mat &cells, int levels = 32) {
//     levels = std::max(2, std::min(256, levels));
//     const float scale = static_cast<float>(levels - 1);
//     // Work in-place, processing left-to-right, top-to-bottom.
//     for (int y = 0; y < cells.rows; ++y) {
//         float *row = cells.ptr<float>(y);
//         float *rowDown = (y + 1 < cells.rows) ? cells.ptr<float>(y + 1) : nullptr;
//         for (int x = 0; x < cells.cols; ++x) {
//             float oldV = std::clamp(row[x], 0.0f, 1.0f);
//             float q = std::round(oldV * scale) / scale; // quantized luminance
//             row[x] = q;
//             float err = oldV - q;
//             // Distribute error to neighbors (classic FS weights)
//             if (x + 1 < cells.cols) row[x + 1] += err * (7.0f / 16.0f);
//             if (rowDown) {
//                 if (x > 0) rowDown[x - 1] += err * (3.0f / 16.0f);
//                 rowDown[x] += err * (5.0f / 16.0f);
//                 if (x + 1 < cells.cols) rowDown[x + 1] += err * (1.0f / 16.0f);
//             }
//         }
//     }
//     // Clamp once after diffusion to keep values in range
//     for (int y = 0; y < cells.rows; ++y) {
//         float *row = cells.ptr<float>(y);
//         for (int x = 0; x < cells.cols; ++x) {
//             row[x] = std::clamp(row[x], 0.0f, 1.0f);
//         }
//     }
// }

static void applyGamma(cv::Mat &cells, double gamma) {
    if ((!(gamma > 0.0)) || std::abs(gamma - 1.0) < 1e-6) {
        return;
    }
    const double inv = 1.0 / gamma;
    cv::pow(cells, inv, cells);
    // Keep in range just in case of numeric drift
    cv::min(cells, 1.0, cells);
    cv::max(cells, 0.0, cells);
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
    
    cv::Mat cells;
    // int supersampling = std::max(1, params.supersampling_scale);
    // if (supersampling > 1) {
    //     // Supersample: upscale with a smooth kernel, then area-average down
    //     cv::Mat hiGray, downGray;
    //     cv::resize(blurred, hiGray, cv::Size(columns * supersampling, rows * supersampling), 0, 0, cv::INTER_CUBIC);
    //     cv::resize(hiGray, downGray, cv::Size(columns, rows), 0, 0, cv::INTER_AREA);
    //     // Compute per-cell average luminance (in linear light) instead of per-pixel mapping.
    //     computeCellMeans(downGray, columns, rows, cells);
    // } else {
    //     // Fallback: direct area average
        
    //     // Compute per-cell average luminance (in linear light) instead of per-pixel mapping.

    //     // computeCellMeans(blurred, columns, rows, cells);
    // }

    cv::resize(gray, cells, cv::Size(columns, rows), 0, 0, cv::INTER_AREA);
    computeCellMeans(gray, columns, rows, cells);
    cv::GaussianBlur(cells, cells, cv::Size(3, 3), 0.6);

    applyGamma(cells, params.gamma);
    // if (params.dithering == DitheringType::FloydSteinberg) {
    //     applyFloydSteinberg(gray, 32);
    // } else if (params.dithering == DitheringType::Ordered) {
    //     applyOrderedDither(gray);
    // }

    const AsciiMapper mapper(calibrator);
    Result result;
    result.lines.resize(rows);

    for (int row = 0; row < rows; ++row) {
        QString line;
        line.reserve(columns);
        const float *row_ptr = cells.ptr<float>(row);
        for (int col = 0; col < columns; ++col) {
            const double luminance = static_cast<double>(row_ptr[col]);
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
