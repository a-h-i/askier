#pragma once

#include "GlyphDensityCalibrator.hpp"
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>

typedef std::result_of_t<decltype(&GlyphDensityCalibrator::lut)(GlyphDensityCalibrator)> lut_type;


cv::Mat ascii_mapper_ocl(cv::ocl::Context &context, const cv::UMat &src,
                         const std::remove_reference_t<std::remove_const_t<lut_type> > &lut);
