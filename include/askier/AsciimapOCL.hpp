#pragma once

#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>


[[nodiscard]] cv::UMat ascii_mapper_ocl(cv::ocl::Context &context, const cv::UMat &src,
                         const cv::UMat &deviceLut);
