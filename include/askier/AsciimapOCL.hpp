#pragma once

#include "GlyphDensityCalibrator.hpp"
#include <opencv2/core.hpp>

typedef std::result_of_t<decltype(&GlyphDensityCalibrator::lut)(GlyphDensityCalibrator)> lut_type;


cv::Mat asciiMapOCL(const cv::UMat &src, const std::remove_reference_t<std::remove_const_t<lut_type> > &lut);