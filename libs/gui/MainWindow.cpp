#include "gui/MainWindow.hpp"

#include "askier/Constants.hpp"
#include "askier/version.hpp"
#include <QMenuBar>
#include <QSplitter>
#include <QHBoxLayout>
#include <QWidget>
#include <QStatusBar>
#include <QFileDialog>
#include <opencv2/imgcodecs.hpp>
#include <QMessageBox>
#include <chrono>
#include <QFontDialog>
#include <QStandardPaths>
#include <QList>

#include "askier/ImageUtils.hpp"
#include "gui/ConversionParamsDialog.hpp"

static const std::string SWITCH_TO_CAMERA_TEXT = "Switch to camera";
static const std::string SWITCH_TO_IMAGE_TEXT = "Switch to image";

static QPixmap fitPixmap(const QImage &img, const QSize &area) {
    if (img.isNull()) {
        return QPixmap();
    }
    return QPixmap::fromImage(img).scaled(area, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), params{
                                              .columns = 120,
                                              .gamma = 10.0,
                                              .dithering = DitheringType::None,
                                              .font = QFont("Monospace", DEFAULT_FONT_SIZE),
                                              .supersampling_scale = 2,
                                          } {
    params.font.setStyleHint(QFont::Monospace);
    setupUi();
    ensureCalibrator();
    pipeline = std::make_unique<AsciiPipeline>(calibrator);
    if (mode == InputMode::Camera) {
        startCamera();
    }
}

MainWindow::~MainWindow() {
    stopCamera();
}

void MainWindow::setupUi() {
    setWindowTitle(ASKIER_NAME);
    resize(1200, 800);

    auto *fileMenu = menuBar()->addMenu("&File");
    actToggleMode = new QAction(SWITCH_TO_IMAGE_TEXT.c_str(), this);
    connect(actToggleMode, &QAction::triggered, this, &MainWindow::onToggleMode);

    actOpenImage = new QAction("Open image", this);
    connect(actOpenImage, &QAction::triggered, this, &MainWindow::onOpenImage);
    actOpenImage->setEnabled(false);

    actSaveAscii = new QAction("Save ASCII", this);
    connect(actSaveAscii, &QAction::triggered, this, &MainWindow::onSaveAscii);

    actChooseFont = new QAction("Choose font", this);
    connect(actChooseFont, &QAction::triggered, this, &MainWindow::onFontChanged);

    actAdjustParams = new QAction("Adjust parameters", this);
    connect(actAdjustParams, &QAction::triggered, this, &MainWindow::onAdjustParams);

    fileMenu->addAction(actToggleMode);
    fileMenu->addAction(actOpenImage);
    fileMenu->addSeparator();
    fileMenu->addAction(actSaveAscii);
    fileMenu->addSeparator();
    fileMenu->addAction(actChooseFont);
    fileMenu->addAction(actAdjustParams);

    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    auto *splitter = new QSplitter(Qt::Horizontal, central);

    originalView = new QLabel("Original");
    originalView->setAlignment(Qt::AlignCenter);
    originalView->setMinimumSize(200, 200);

    asciiView = new QLabel("ASCII Preview");
    asciiView->setAlignment(Qt::AlignCenter);
    asciiView->setMinimumSize(200, 200);

    splitter->addWidget(originalView);
    splitter->addWidget(asciiView);
    layout->addWidget(splitter);

    setCentralWidget(central);
    statusBar()->showMessage("Ready");
}

void MainWindow::ensureCalibrator() {
    calibrator = std::make_shared<GlyphDensityCalibrator>(params.font);
    calibrator->ensureCalibrated();
}

void MainWindow::startCamera() {
    if (captureWorker) {
        return;
    }
    captureWorker = std::make_unique<VideoCaptureWorker>(0);
    connect(captureWorker.get(), &VideoCaptureWorker::frameCaptured, this, &MainWindow::onFrameCaptured);
    captureWorker->start();
    statusBar()->showMessage("Live capture mode");
}

void MainWindow::stopCamera() {
    if (!captureWorker) {
        return;
    }
    captureWorker->stop();
    captureWorker->wait();
    captureWorker.reset();
}

void MainWindow::onToggleMode() {
    if (mode == Camera) {
        mode = ImageFile;
        actToggleMode->setText(SWITCH_TO_CAMERA_TEXT.c_str());
        actOpenImage->setEnabled(true);
        stopCamera();
        statusBar()->showMessage("Image mode");
    } else {
        mode = Camera;
        actToggleMode->setText(SWITCH_TO_IMAGE_TEXT.c_str());
        actOpenImage->setEnabled(false);
        startCamera();
        statusBar()->showMessage("Live capture mode");
    }
}

void MainWindow::onOpenImage() {
    const auto path = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
    const QString file = QFileDialog::getOpenFileName(this, "Open Image",
                                                      path,
                                                      "Images (*.png *.jpg *.jpeg *.bmp *.webp)");
    if (file.isEmpty()) {
        return;
    }
    cv::Mat bgr = cv::imread(file.toStdString(), cv::IMREAD_COLOR);
    if (bgr.empty()) {
        QMessageBox::warning(this, "Open Image", "Failed to load image.");
        return;
    }
    stillBgr = bgr;
    refreshAsciiFromStill();
}

void MainWindow::refreshAsciiFromStill() {
    if (stillBgr.empty()) {
        return;
    }
    lastOriginalImage = matToQImage(stillBgr);
    originalView->setPixmap(fitPixmap(lastOriginalImage, originalView->size()));
    runAsciiPipeline(stillBgr);
}


void MainWindow::onFrameCaptured(const cv::Mat &frame) {
    if (mode != Camera) {
        return;
    }
    lastOriginalImage = matToQImage(frame);
    originalView->setPixmap(fitPixmap(lastOriginalImage, originalView->size()));
    runAsciiPipeline(frame);
}

void MainWindow::runAsciiPipeline(const cv::Mat &bgr) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    const auto before = high_resolution_clock::now();
    // run pipeline
    auto result = pipeline->process(bgr, params);
    lastAsciiLines = std::move(result.lines);
    asciiView->setPixmap(fitPixmap(result.preview, asciiView->size()));
    const auto after = high_resolution_clock::now();
    const auto elapsed_ms = duration_cast<milliseconds>(after - before);
    statusBar()->showMessage(QString("Generated ASCII preview in %1ms").arg(elapsed_ms.count()));
}

void MainWindow::onSaveAscii() {
    if (lastAsciiLines.empty()) {
        QMessageBox::information(this, "Save ASCII", "Nothing to save yet.");
        return;
    }
    QString path = QFileDialog::getSaveFileName(this, "Save ASCII", {}, "Text files (*.txt)");
    if (path.isEmpty()) {
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(this, "Save ASCII", "Failed to write file.");
        return;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    for (const auto &line: lastAsciiLines) {
        out << line << '\n';
    };
    file.close();
    statusBar()->showMessage("Saved ASCII to " + path, 3000);
}

void MainWindow::onFontChanged() {
    bool ok = false;
    QFont chosen = QFontDialog::getFont(&ok, params.font, this, "Choose monospace font");
    if (!ok) {
        return;
    }
    params.font = chosen;
    ensureCalibrator();
    pipeline = std::make_unique<AsciiPipeline>(calibrator);
    if (mode == ImageFile) {
        refreshAsciiFromStill();
    }
}


void MainWindow::onAdjustParams() {
    ConversionParamsDialog dialog(params, this);
    if (dialog.exec() == QDialog::Accepted) {
        params = dialog.getParams();
        ensureCalibrator();
        pipeline = std::make_unique<AsciiPipeline>(calibrator);
        if (mode == ImageFile) {
            refreshAsciiFromStill();
        }
    }
}
