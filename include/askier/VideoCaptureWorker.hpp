#pragma once
#include <atomic>
#include <QThread>
#include <opencv2/core.hpp>

class VideoCaptureWorker : public QThread {
    Q_OBJECT

public:
    explicit VideoCaptureWorker(int device_index = 0, QObject *parent = nullptr);

    ~VideoCaptureWorker();

    void stop();

    signals:
    

    void frameCaptured(const cv::Mat &frame);

protected:
    void run() override;

private:
    int device_index;
    std::atomic<bool> is_running{false};
};