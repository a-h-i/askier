#pragma once

#include <opencv2/core/mat.hpp>
#include <opencv2/core/ocl.hpp>

/**
* Applies an ordered dithering effect to the given matrix of grayscale cell values
 * using a 4x4 Bayer matrix. The resulting matrix values will be adjusted to add a
 * dithering effect while preserving their range within [0,1].
 * https://en.wikipedia.org/wiki/Ordered_dithering
 * @param cells A cv::UMat of type CV_32FC1 (single-channel 32-bit floating point)
 *              containing grayscale values normalized to the range [0,1].
 *
 */
void applyOrderedDither(cv::UMat &cells);

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
 * @param cells A cv::UMat representing the grayscale image; pixel values must
 *              be in the range [0.0, 1.0]. The matrix is modified in-place.
 * @param levels The number of quantization levels, clamped to the range [2, 256].
 *               Defaults to 32.
 * @param context opencl context
 */
void applyFloydSteinberg(cv::ocl::Context &context, cv::UMat &cells, int levels = 32);