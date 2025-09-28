#include <string>
#include <opencv2/core/ocl.hpp>

#include "askier/Dithering.hpp"

static std::string fs_kernel_src = R"SRC(

kernel void floyd_steinberg_serpentine(
    __global float *img,
    int rows,
    int cols,
    int levels
) {
    int y = get_global_id(0);
    float invScale = 1.0f / (float)(levels - 1);
    // Serpentine: left->right on even rows, right->left on odd rows
    int dir = (y & 1) ? -1 : 1;
    int xStart = (dir == 1) ? 0 : (cols - 1);
    int xEndExclusive = (dir == 1) ? cols : -1;

    for (int x = xStart; x != xEndExclusive; x += dir) {
        int idx = y * cols + x;
        float v = fmin(fmax(img[idx], 0.0f), 1.0f);
        // Quantize
        float q = round(v / invScale) * invScale;
        q = fmin(fmax(q, 0.0f), 1.0f);
        img[idx] = q;
        float err = v - q;
        if (err == 0.0f) continue;

        // Distribute error
        int xn = x + dir;
        if (xn >= 0 && xn < cols) {
            img[y * cols + xn] += err * (7.0f / 16.0f);
        }
        int y1 = y + 1;
        if (y1 < rows) {
            int xl = x - dir; // opposite side because of serpentine
            if (xl >= 0 && xl < cols) {
                img[y1 * cols + xl] += err * (3.0f / 16.0f);
            }
            img[y1 * cols + x] += err * (5.0f / 16.0f);
            if (xn >= 0 && xn < cols) {
                img[y1 * cols + xn] += err * (1.0f / 16.0f);
            }
        }
    }

}

)SRC";

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
 */
void applyFloydSteinberg(cv::UMat &cells, int levels) {
    levels = std::clamp(levels, 2, 256);
    CV_Assert(cells.type() == CV_32F && cells.channels() == 1);
    // Ensure we have a contiguous buffer in row-major cols stride
    cv::UMat continous;
    if (cells.isContinuous() && cells.cols == static_cast<int>(cells.step1())) {
        continous = cells;
    } else {
        cells.copyTo(continous);
    }

    cv::UMat flat = continous.reshape(1, 1);
    CV_Assert(flat.isContinuous());

    auto context = cv::ocl::Context::getDefault();
    std::string compileErrors;
    cv::ocl::ProgramSource source(fs_kernel_src);
    cv::ocl::Program program = context.getProg(source, "", compileErrors);
    if (program.empty()) {
        throw std::runtime_error("OpenCL ascii mapper compilaton failed" + compileErrors);
    }
    cv::ocl::Kernel kernel("floyd_steinberg_serpentine", program);
    CV_Assert(!kernel.empty());
    // Kernel args
    // Pass the original 2D buffer but index as flat (rows*cols). We use reshape to ensure continuity.
    kernel.args(
        cv::ocl::KernelArg::ReadWrite(continous), // underlying buffer shared with uFlat
        cells.rows,
        cells.cols,
        levels
    );
    // Global size: one work-item per row (row-serial, in-row sequential)
    size_t global[1] = { static_cast<size_t>(cells.rows) };
    const bool ok = kernel.run(1, global, nullptr, true);
    CV_Assert(ok);

}
