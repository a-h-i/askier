#include <opencv2/core.hpp>

#include "askier/Dithering.hpp"

/**
* Applies an ordered dithering effect to the given matrix of grayscale cell values
 * using a 4x4 Bayer matrix. The resulting matrix values will be adjusted to add a
 * dithering effect while preserving their range within [0,1].
 * https://en.wikipedia.org/wiki/Ordered_dithering
 * @param cells A cv::UMat of type CV_32FC1 (single-channel 32-bit floating point)
 *              containing grayscale values normalized to the range [0,1].
 *
 */
 void applyOrderedDither(cv::UMat &cells) {
    // Bayer 4x4 pattern as CV_32F
    static const float bayerData[16] = {
        0, 8, 2, 10,
        12, 4, 14, 6,
        3, 11, 1, 9,
        15, 7, 13, 5
    };
    cv::Mat bayer4x4(4, 4, CV_32F, const_cast<float*>(bayerData));
    // Normalize to [0,1): t = b/16, bias around zero by subtracting 0.5, scale strength.
    constexpr float invN = 1.0f / 16.0f;
    cv::UMat bayer;
    bayer4x4.convertTo(bayer, CV_32F, invN);
    cv::UMat tiled;
    cv::repeat(bayer, (cells.rows + 3) / 4, (cells.cols + 3) / 4, tiled);
    tiled = tiled(cv::Rect(0, 0, cells.cols, cells.rows));
    constexpr float strength = 1.0f / 16.0f; // smaller = subtler pattern
    cv::UMat bias;
    cv::subtract(tiled, 0.5f, bias);
    cv::multiply(bias, strength, bias);
    cv::add(cells, bias, cells);
    // Clamp to [0,1]
    cv::min(cells, 1.0f, cells);
    cv::max(cells, 0.0f, cells);
}