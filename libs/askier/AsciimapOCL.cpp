#include "askier/AsciimapOCL.hpp"

#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/ocl.hpp>

#include "askier/GlyphDensityCalibrator.hpp"


static std::string kernel_source = R"SRC(
kernel void ascii_map_lut(
__global const float *src,
int src_step,
int src_offset,
int src_rows,
int src_cols,
__global const uchar *lut,
int lut_size,
__global uchar *dst,
int dst_step,
int dst_offset,
int dst_rows,
int dst_cols
)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);


    const int source_idx = y * src_step / sizeof(float)  + x + src_offset ;
    const int dst_idx = y * dst_step + dst_offset + x;

    const float luminance = src[source_idx];
    const float darkness = 1.0f - luminance;

    const int max_lut_index = lut_size - 1;
    int darkness_index = (int) round(darkness * max_lut_index);
    if (darkness_index < 0) {
        darkness_index = 0;
    } if (darkness_index > max_lut_index) {
        darkness_index = max_lut_index;
    }
    dst[dst_idx] = lut[darkness_index];
}
)SRC";

cv::UMat ascii_mapper_ocl(cv::ocl::Context &context, const cv::UMat &src,
                         const cv::UMat &deviceLut) {
    CV_Assert(src.type() == CV_32F);
    CV_Assert(deviceLut.type() == CV_8UC1);
    CV_Assert(deviceLut.rows == 1);
    CV_Assert(deviceLut.cols == ASCII_COUNT);
    cv::UMat dst(src.size(), CV_8UC1, cv::USAGE_ALLOCATE_DEVICE_MEMORY);

    cv::ocl::ProgramSource source(kernel_source);
    std::string compileErrors;
    cv::ocl::Program program = context.getProg(source, "", compileErrors);
    if (program.empty()) {
        throw std::runtime_error("OpenCL ascii mapper compilation failed" + compileErrors);
    }
    cv::ocl::Kernel kernel("ascii_map_lut", program);
    CV_Assert(!kernel.empty());
    CV_Assert(src.isContinuous());
    CV_Assert(deviceLut.isContinuous());
    CV_Assert(dst.isContinuous());
    CV_Assert(!deviceLut.empty());

    kernel.args(
        cv::ocl::KernelArg::ReadOnly(src),
        cv::ocl::KernelArg::PtrReadOnly(deviceLut),
        ASCII_COUNT,
        cv::ocl::KernelArg::WriteOnly(dst)
    );


    size_t globals[2] = {static_cast<size_t>(src.cols), static_cast<size_t>(src.rows)};
    bool run_ok = kernel.run(2, globals, nullptr, true);
    CV_Assert(run_ok);

    return dst;
}