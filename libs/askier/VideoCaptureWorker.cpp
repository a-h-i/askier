#include "askier/VideoCaptureWorker.hpp"
#include <opencv2/opencv.hpp>

VideoCaptureWorker::VideoCaptureWorker(int device_index, QObject *parent) : QThread(parent),
                                                                            device_index(device_index) {
}

VideoCaptureWorker::~VideoCaptureWorker() {
    stop();
    wait();
}

void VideoCaptureWorker::stop() {
    is_running = false;
}

void VideoCaptureWorker::run() {
    cv::VideoCapture cap(device_index);
    if (!cap.isOpened()) {
        return;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    is_running = true;
    cv::Mat frame;
    while (is_running) {
        if (!cap.read(frame)) {
            break;
        }
        cv::flip(frame, frame, 1); // Mirror the frame
        emit frameCaptured(frame.clone());
        msleep(16); // 60 fps
    }
}