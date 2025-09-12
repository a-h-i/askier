#include "askier/ImageUtils.hpp"

#include <opencv2/imgproc.hpp>


QImage matToQImage(const cv::Mat &bgr) {
    if (bgr.empty()) {
        return QImage();
    }
    cv::Mat rgb;
    cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
    return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
}