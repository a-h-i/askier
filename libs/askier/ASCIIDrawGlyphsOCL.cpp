#include <opencv2/opencv.hpp>
#include <string>
#include <CL/opencl.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/ocl.hpp>


static std::string kernel_source = R"SRC(
kernel void ascii_map_glyphs(
    __global const uchar *glyphs,
    int glyphs_step,
    int glyphs_offset,
    int glyphs_rows,
    int glyphs_cols,
    __global const uchar *dense_pixmaps,
    int pixmap_width,
    int pixmap_height,
    __global uchar *dst,
    int dst_step,
    int dst_offset,
    int dst_rows,
    int dst_cols
) {
     const int x = get_global_id(0);
     const int y = get_global_id(1);
     const int glyph_idx = y * glyphs_step + x + glyphs_offset;
     const uchar glyph = glyphs[glyph_idx];
     int pixmap_glyph_idx = glyph - 32;
     if (pixmap_glyph_idx < 0) {
        return;
     }
     const int glyph_area = pixmap_height * pixmap_width;
     const int pixmap_start_offset = pixmap_glyph_idx * glyph_area;
     const int dst_pixel_y = y * pixmap_height;
     const int dst_pixel_x = x * pixmap_width;
     for(int pmap_y = 0; pmap_y < pixmap_height; ++pmap_y) {
         const int dst_y = dst_pixel_y + pmap_y;
         for(int pmap_x = 0; pmap_x < pixmap_width; ++pmap_x) {
             const int dst_x = dst_pixel_x + pmap_x;
             const int dst_idx = dst_y * dst_cols + dst_x;
             const int pmap_idx = pixmap_start_offset + pmap_y * pixmap_width + pmap_x ;
             dst[dst_idx] = dense_pixmaps[pmap_idx];
         }
     }
}
)SRC";

cv::Mat ascii_draw_glyphs_ocl(
    cv::ocl::Context &context,
    const cv::UMat &glyphs,
    const cv::UMat &densePixmaps,
    const int pixmapWidth,
    const int pixmapHeight,
    const int outputCellWidth,
    const int outputCellHeight
) {
    CV_Assert(glyphs.type() == CV_8U);
    CV_Assert(densePixmaps.type() == CV_8U);
    CV_Assert(outputCellWidth >= 1);
    CV_Assert(outputCellHeight >= 1);
    const int dstCols = glyphs.cols * outputCellWidth;
    const int dstRows = glyphs.rows * outputCellHeight;

    cv::UMat dst(cv::Size(dstCols, dstRows), CV_8UC1,
                 cv::USAGE_ALLOCATE_DEVICE_MEMORY);
    dst.setTo(0);

    cv::ocl::ProgramSource source(kernel_source);
    std::string compileErrors;
    cv::ocl::Program program = context.getProg(source, "", compileErrors);
    if (program.empty()) {
        throw std::runtime_error("OpenCL ascii draw glyphs compilation failed" + compileErrors);
    }
    cv::ocl::Kernel kernel("ascii_map_glyphs", program);
    CV_Assert(!kernel.empty());
    CV_Assert(densePixmaps.isContinuous());
    CV_Assert(dst.isContinuous());
    kernel.args(
        cv::ocl::KernelArg::ReadOnly(glyphs),
        cv::ocl::KernelArg::PtrReadOnly(densePixmaps),
        pixmapWidth,
        pixmapHeight,
        cv::ocl::KernelArg::WriteOnly(dst)
    );
    size_t globals[2] = {static_cast<size_t>(glyphs.cols), static_cast<size_t>(glyphs.rows)};
    bool run_ok = kernel.run(2, globals, nullptr, true);
    CV_Assert(run_ok);
    return dst.getMat(cv::ACCESS_READ).clone();
}
