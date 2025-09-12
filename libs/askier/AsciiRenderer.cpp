#include "askier/AsciiRenderer.hpp"
#include "askier/Constants.hpp"
#include <QFontMetrics>
#include <QPainter>


AsciiRenderer::AsciiRenderer(const QFont &font) : font(font) {
    if (this->font.pointSize() <= 0) {
        this->font.setPointSize(DEFAULT_FONT_SIZE);
    }
}

QImage AsciiRenderer::render(const std::vector<QString> &lines) const {
    if (lines.empty()) {
        return QImage();
    }
    const QFontMetrics metrics(font);
    const int cell_width = std::max(10, metrics.horizontalAdvance("M"));
    const int cell_height = std::max(10, metrics.lineSpacing());
    const int columns = std::ranges::max_element(lines.begin(), lines.end(), [](const QString &a, const QString &b) {
        return a.size() < b.size();
    })->size();
    const int rows = lines.size();

    QImage img(columns * cell_width, rows * cell_height, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter painter(&img);
    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing, true);
    painter.setPen(Qt::black);
    painter.setFont(font);

    for (int row = 0; row < rows; row++) {
        painter.drawText(0, row * cell_height, columns * cell_width, cell_height, Qt::AlignLeft, lines[row]);
    }
    painter.end();
    return img;
}


