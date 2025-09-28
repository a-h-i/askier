#include <string>
#include <CL/opencl.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/ocl.hpp>


std::string kernel_source = R"SRC(

kernel void ascii_map_glyphs(
__global const uchar *glyphs,
int glyphs_step,
int glyphs_offset,
__global const uchar *dense_pixmaps,
__global const uchar *pixmap_widths,
__global const uchar *pixmap_heights,
__global uchar *dst,
int output_cell_width,
int output_cell_height,
) {
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int glyph_idx = y * glyphs_step + x + glyphs_offset;
    const uchar glyph = glyphs[glyph_idx];
    const width = pixmap_widths[(int)glyph];
    const height = pixmap_heights[(int)glyph];
    // We have a pixmap representing glyph of width x height dimensions
    // that we want to render into a cell of output_cell_width x output_cell_height dimensions.
    // This is further complicated by needing to find the start index of our pixmap in dense_pixmaps
    // as they are of non-uniform dimensions.
}

)SRC";

cv::Mat ascii_draw_glyphs_ocl(
    cv::ocl::Context &context,
    const cv::UMat &glyphs,
    const cv::UMat &densePixmaps,
    const cv::UMat &pixmapWidths,
    const cv::UMat &pixmapHeights,
    const int outputCellWidth,
    const int outputCellHeight
) {
    CV_Assert(glyphs.type() == CV_8U);
    CV_Assert(densePixmaps.type() == CV_8U);
    CV_Assert(pixmapWidths.type() == CV_8U);
    CV_Assert(pixmapHeights.type() == CV_8U);
    CV_Assert(outputCellWidth >= 1);
    CV_Assert(outputCellHeight >= 1);

    cv::UMat dst(glyphs.rows * outputCellHeight, glyphs.cols * outputCellWidth, CV_8UC1,
                 cv::USAGE_ALLOCATE_DEVICE_MEMORY);

    cv::ocl::ProgramSource source(kernel_source);
    std::string compileErrors;
    cv::ocl::Program program = context.getProg(source, "", compileErrors);
    if (program.empty()) {
        throw std::runtime_error("OpenCL ascii draw glyphs compilation failed" + compileErrors);
    }
    cv::ocl::Kernel kernel("ascii_map_glyphs", program);
    CV_Assert(!kernel.empty());
    CV_Assert(densePixmaps.isContinuous());
    CV_Assert(pixmapWidths.isContinuous());
    CV_Assert(pixmapHeights.isContinuous());
    CV_Assert(dst.isContinuous());

    kernel.args(
        cv::ocl::KernelArg::ReadOnly(glyphs),
        cv::ocl::KernelArg::PtrReadOnly(densePixmaps),
        cv::ocl::KernelArg::PtrReadOnly(pixmapWidths),
        cv::ocl::KernelArg::PtrReadOnly(pixmapHeights),
        cv::ocl::KernelArg::WriteOnly(dst),
        outputCellWidth,
        outputCellHeight
    );

    size_t globals[2] = {static_cast<size_t>(glyphs.cols), static_cast<size_t>(glyphs.rows)};
    bool run_ok = kernel.run(2, globals, nullptr, true);
    CV_Assert(run_ok);
    return dst.getMat(cv::ACCESS_READ).clone();
}
