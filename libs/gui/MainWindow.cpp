#include "gui/MainWindow.hpp"

#include "askier/Constants.hpp"


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
