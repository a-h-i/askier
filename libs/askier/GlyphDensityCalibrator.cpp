#include "askier/GlyphDensityCalibrator.hpp"

#include <iostream>

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
#include <QImageWriter>


static QString cacheBasePath() {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    return base;
}


static QString fontKey(const QFont &font) {
    return font.family() + QString("_%1").arg(font.pointSize());
}

static QString glyphPixmapCachePath(const QFont &font, const QString &glyph, const QString &extension) {
    const auto base = cacheBasePath() + "/pixmaps";
    const QString key = fontKey(font);
    QDir().mkpath(base);
    std::string glyphCode = std::to_string(static_cast<int>(glyph.at(0).toLatin1()));
    return base + "/" + ASKIER_VERSION + "_" + key + "_glyph_" + QString::fromStdString(glyphCode) + "_pixmap" + "." + extension;
}


static QString cachePathFromFont(const QFont &font) {
    const auto base = cacheBasePath();

    const QString key = fontKey(font);
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
        std::vector<unsigned char> pixmap;
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
        const QString s{QChar(c)};
        const int x = 0;
        int y = cell_height - metrics.descent();
        y = std::clamp(y, 0, cell_height);
        painter.drawText(x, y, s);
        painter.end();
        const auto imgPath = glyphPixmapCachePath(font_, s,"png");
        QImageWriter writer(imgPath);
        const auto saved = writer.write(img);
        if (!saved) {
            throw std::runtime_error("Error saving glyph" + s.toStdString() + " pixmap:\n" + writer.errorString().toStdString());
        }

        // compute normalized darkness coverage
        double sum = 0.0;
        std::vector<unsigned char> pixmap;
        pixmap.reserve(cell_height * cell_width);
        for (int row = 0; row < cell_height; ++row) {
            for (int column = 0; column < cell_width; ++column) {
                const auto pixel = img.pixel(column, row);
                const unsigned char gray = qGray(pixel);
                pixmap.push_back(gray);
                sum += 1.0 - static_cast<float>(gray) / 255.0;
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
