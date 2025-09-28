#pragma once
#include <opencv2/core/mat.hpp>
#include <opencv2/core/ocl.hpp>


[[nodiscard]] cv::Mat ascii_draw_glyphs_ocl(
    cv::ocl::Context &context,
    const cv::UMat &glyphs,
    const cv::UMat &densePixmaps,
    const cv::UMat &pixmapWidths,
    const cv::UMat &pixmapHeights
);
