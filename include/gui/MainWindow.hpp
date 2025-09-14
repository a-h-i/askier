#pragma once
#include <QMainWindow>
#include <opencv2/core/mat.hpp>
#include <QLabel>

#include "askier/AsciiPipeline.hpp"
#include "askier/GlyphDensityCalibrator.hpp"
#include "askier/VideoCaptureWorker.hpp"


enum InputMode {
    Camera,
    ImageFile,
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:
    void onToggleMode();

    void onOpenImage();

    void onSaveAscii();

    void onFrameCaptured(const cv::Mat &frame);

    void refreshAsciiFromStill();

    void onFontChanged();

    void onAdjustParams();

private:
    void setupUi();

    void startCamera();

    void stopCamera();

    void ensureCalibrator();

    void runAsciiPipeline(const cv::Mat &bgr);

    // ui
    QLabel *originalView = nullptr;
    QLabel *asciiView = nullptr;
    QAction *actToggleMode = nullptr;
    QAction *actOpenImage = nullptr;
    QAction *actSaveAscii = nullptr;
    QAction *actChooseFont = nullptr;
    QAction *actAdjustParams = nullptr;
    // state
    InputMode mode = InputMode::Camera;
    QImage lastOriginalImage;
    std::vector<QString> lastAsciiLines;

    // Engine
    std::unique_ptr<VideoCaptureWorker> captureWorker;
    std::shared_ptr<GlyphDensityCalibrator> calibrator;
    std::unique_ptr<AsciiPipeline> pipeline;
    AsciiParams params;

    // Cache for still image processing
    cv::Mat stillBgr;
};
