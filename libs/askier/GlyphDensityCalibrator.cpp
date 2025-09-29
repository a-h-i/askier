#include "askier/GlyphDensityCalibrator.hpp"
#include "askier/Constants.hpp"
#include "askier/version.hpp"
#include <QStandardPaths>
#include <QString>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>


QString cachePathFromFont(const QFont &font) {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    const QString key = font.family() + QString("_%1").arg(font.pointSize());
    const QString path = base + "/ascii_lut_v" + ASKIER_VERSION + "_" + key + ".json";
    return path;
}

GlyphDensityCalibrator::GlyphDensityCalibrator(const QFont &font) : font_(font) {
    if (font_.pointSize() <= 0) {
        font_.setPointSize(DEFAULT_FONT_SIZE);
    }
}

void GlyphDensityCalibrator::ensureCalibrated() {
    if (tryLoadCache()) {
        return;
    }
    calibrate();
    saveCache();
}

bool GlyphDensityCalibrator::tryLoadCache() {
    QString path = cachePathFromFont(font_);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    auto doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        return false;
    }
    auto obj = doc.object();
    auto arr = obj.value("lut").toArray();
    auto pixmaps = obj.value("pixmap").toArray();
    auto heights = obj.value("pixmap_heights").toArray();
    auto widths = obj.value("pixmap_widths").toArray();

    if (arr.size() != ASCII_COUNT || heights.size() != ASCII_COUNT || widths.size() !=
        ASCII_COUNT) {
        return false;
    }

    for (int i = 0; i < ASCII_COUNT; ++i) {
        lut_[i] = static_cast<char>(arr[i].toInt());
        pixmap_widths[i] = widths[i].toInt();
        pixmap_heights[i] = heights[i].toInt();
    }
    this->pixmaps_.reserve(pixmaps.size());
    for (int i = 0; i < pixmaps.size(); ++i) {
        this->pixmaps_.push_back(static_cast<uchar>(pixmaps[i].toInt()));
    }
    aspect = obj.value("aspect").toDouble(2.0);
    return true;
}

void GlyphDensityCalibrator::saveCache() {
    QString path = cachePathFromFont(font_);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    QJsonObject obj;
    QJsonArray arr;
    QJsonArray heights;
    QJsonArray widths;
    for (int i = 0; i < ASCII_COUNT; ++i) {
        arr.append(static_cast<int>(lut_[i]));

        heights.append(pixmap_heights[i]);
        widths.append(pixmap_widths[i]);
    }
    QJsonArray pixMap;
    for (const auto &byte: pixmaps_) {
        pixMap.append(byte);
    }
    obj.insert("lut", arr);
    obj.insert("aspect", aspect);
    obj.insert("pixmap", pixMap);
    obj.insert("pixmap_heights", heights);
    obj.insert("pixmap_widths", widths);
    file.write(QJsonDocument(obj).toJson());
    file.close();
}

void GlyphDensityCalibrator::calibrate() {
    QFontMetrics metrics(font_);
    const int cell_width = std::max(10, metrics.horizontalAdvance("M"));
    const int cell_height = std::max(10, metrics.height());

    aspect = static_cast<double>(cell_height) / static_cast<double>(cell_width);

    // Render each printable ASCII glyph on a white background and measure ink coverage;
    struct GlyphDensity {
        char c;
        double density;
        std::vector<uchar> pixmap;
        int cell_width, cell_height;
    };
    std::vector<GlyphDensity> glyphs;
    glyphs.reserve(ASCII_COUNT);


    for (int c = ASCII_MIN; c <= ASCII_MAX; ++c) {
        QImage img(cell_width, cell_height, QImage::Format_Grayscale8);
        img.fill(255);
        QPainter painter(&img);
        painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing, true);
        painter.setPen(Qt::black);
        painter.setFont(font_);
        const QString s = QString(QChar(c));
        const auto boundingRect = metrics.tightBoundingRect(s);
        int x = cell_width - boundingRect.width();
        x = std::clamp(x, 0, cell_width / 2);
        int baselineY = cell_height - boundingRect.height() / 2;
        baselineY = std::clamp(baselineY, 0, cell_height / 2);
        painter.drawText(x, baselineY, s);
        painter.end();

        // compute normalized darkness coverage
        const uchar *bits = img.constBits();
        const size_t stride = img.bytesPerLine();
        double sum = 0.0;
        std::vector<uchar> pixmap;
        pixmap.resize(cell_height * cell_width);
        for (int y = 0; y < cell_height; ++y) {
            const uchar *row = bits + y * stride;
            for (int column = 0; column < cell_width; ++column) {
                const double gray = row[column];
                pixmap.push_back(row[column]);
                sum += 1.0 - gray / 255.0;
            }
        }
        const double density = sum / (cell_width * cell_height);
        glyphs.push_back({static_cast<char>(c), density, pixmap, cell_width, cell_height});
    }
    std::ranges::sort(glyphs, [](const GlyphDensity &a, const GlyphDensity &b) {
        return a.density < b.density;
    });
    // build LUT
    for (size_t i = 0; i < lut_.size(); ++i) {
        const auto &glyph = glyphs[i];
        lut_[i] = glyph.c;
        std::ranges::copy(glyph.pixmap, std::back_inserter(pixmaps_));
        pixmap_heights[i] = glyph.cell_height;
        pixmap_widths[i] = glyph.cell_width;
    }
}
