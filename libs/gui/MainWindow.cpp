#include "gui/MainWindow.hpp"

#include "askier/Constants.hpp"
#include <QMenuBar>
#include "askier/version.hpp"

static QPixmap fitPixmap(const QImage &img, const QSize &area) {
    if (img.isNull()) {
        return QPixmap();
    }
    return QPixmap::fromImage(img).scaled(area, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), params{
                                              .columns = 120,
                                              .gamma = 1.0,
                                              .dithering = DitheringType::None,
                                              .font = QFont("Monospace", DEFAULT_FONT_SIZE),
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

    auto* fileMenu = menuBar()->addMenu("&File");
    actToggleMode = new QAction("Toggle mode", this);
    connect(actToggleMode, &QAction::triggered, this, &MainWindow::onToggleMode);

    actOpenImage = new QAction("Open image", this);
    connect(actOpenImage, &QAction::triggered, this, &MainWindow::onOpenImage);
    actOpenImage->setEnabled(false);

    actSaveAscii = new QAction("Save ASCII", this);
    connect(actSaveAscii, &QAction::triggered, this, &MainWindow::onSaveAscii);

    actChooseFont = new QAction("Choose font", this);
    connect(actChooseFont, &QAction::triggered, this, &MainWindow::onFontChanged);

    fileMenu->addAction(actToggleMode);
    fileMenu->addAction(actOpenImage);
    fileMenu->addSeparator();
    fileMenu->addAction(actSaveAscii);
    fileMenu->addSeparator();
    fileMenu->addAction(actChooseFont);





}