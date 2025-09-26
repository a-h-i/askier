#pragma once

#include <QImage>
#include <opencv2/core.hpp>

QImage matToQImage(const cv::Mat &bgr);

QImage matToQImageGray(const cv::Mat &gray);