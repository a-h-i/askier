#pragma once

#include <opencv2/core/mat.hpp>


void applyOrderedDither(cv::UMat &cells);
void applyFloydSteinberg(cv::UMat &cells, int levels = 32);