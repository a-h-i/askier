#include "askier/ImageUtils.hpp"

#include <opencv2/imgproc.hpp>


QImage matToQImage(const cv::Mat &bgr) {
    if (bgr.empty()) {
        return QImage();
    }

    return QImage(bgr.data, bgr.cols, bgr.rows, bgr.step, QImage::Format_BGR888).copy();
}