#include <opencv2/core/base.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <CL/opencl.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/ocl.hpp>


static std::string kernel_source = R"SRC(
kernel void ascii_map_glyphs(
    __global const uchar *glyphs,
    __global const uchar *dense_pixmaps,
    __global uchar *dst,
    int pixmap_width,
    int pixmap_height,
    int glyphs_cols,
    int glyphs_rows,
    int dst_cols
) {
     const size_t x = get_global_id(0);
     const size_t y = get_global_id(1);
     if(x >= glyphs_cols || y >= glyphs_rows) {
        return;
    }
    const size_t glyph_idx = y * glyphs_cols + x;
    uchar glyph = glyphs[glyph_idx];
    int pixmap_glyph_idx = glyph - 32;
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

    cv::ocl::ProgramSource source(kernel_source);
    std::string compileErrors;
    cv::ocl::Program program = context.getProg(source, "", compileErrors);
    if (program.empty()) {
        throw std::runtime_error("OpenCL ascii draw glyphs compilation failed" + compileErrors);
    }
    cv::ocl::Kernel kernel("ascii_map_glyphs", program);
    CV_Assert(!kernel.empty());
    CV_Assert(densePixmaps.isContinuous());
    CV_Assert(glyphs.isContinuous());
    CV_Assert(dst.isContinuous());
    int argi = 0;
    kernel.set(argi++, cv::ocl::KernelArg::PtrReadOnly(glyphs));
    kernel.set(argi++, cv::ocl::KernelArg::PtrReadOnly(densePixmaps));
    kernel.set(argi++, cv::ocl::KernelArg::PtrWriteOnly(dst));
    kernel.set(argi++, pixmapWidth);
    kernel.set(argi++, pixmapHeight);
    kernel.set(argi++, glyphs.cols);
    kernel.set(argi++, glyphs.rows);
    kernel.set(argi++, dst.cols);
    size_t globals[2] = {(size_t) glyphs.cols, (size_t) glyphs.rows};
    bool run_ok = kernel.run(2, globals, nullptr, true);
    CV_Assert(run_ok);
    return dst.getMat(cv::ACCESS_READ).clone();
}
