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

    if (arr.size() != 256) {
        return false;
    }

    for (int i = 0; i < 256; ++i) {
        lut_[i] = static_cast<char>(arr[i].toInt());
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
    for (int i = 0; i < 256; ++i) {
        arr.append(static_cast<int>(lut_[i]));
    }
    obj.insert("lut", arr);
    obj.insert("aspect", aspect);
    file.write(QJsonDocument(obj).toJson());
    file.close();
}

void GlyphDensityCalibrator::calibrate() {
    QFontMetrics metrics(font_);
    const int cell_width = std::max(10, metrics.horizontalAdvance("M"));
    const int cell_height = std::max(10, metrics.lineSpacing());
    aspect = static_cast<double>(cell_width) / static_cast<double>(cell_height);

    // Render each printable ASCII glyph on white background and measure ink coverage;
    struct GlyphDensity {
        char c;
        double density;
    };
    std::vector<GlyphDensity> glyphs;
    glyphs.reserve(ASCII_COUNT);

    for (int c = ASCII_MIN; c <= ASCII_MAX; c++) {
        QImage img(cell_width, cell_height, QImage::Format_Grayscale8);
        img.fill(255);
        QPainter painter(&img);
        painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing, true);
        painter.setPen(Qt::black);
        painter.setFont(font_);
        painter.drawText(QRect(0, 0, cell_width, cell_height), Qt::AlignCenter, QString(QChar(c)));
        painter.end();
        // compute normalized darkness coverage
        const uchar *bits = img.constBits();
        const int stride = img.bytesPerLine();
        double sum = 0.0;

        for (int y = 0; y < cell_height; ++y) {
            const uchar *row = bits + y * stride;
            for (int x = 0; x < cell_width; ++x) {
                double gray = static_cast<double>(row[x]);
                sum += (255.0 - gray) / 255.0;
            }
        }
        const double density = sum / (cell_width * cell_height);
        glyphs.push_back({static_cast<char>(c), density});
    }
    std::ranges::sort(glyphs.begin(), glyphs.end(), [](const GlyphDensity &a, const GlyphDensity &b) {
        return a.density < b.density;
    });
    // build LUT
    for (int i = 0; i < 256; i++) {
        double target = static_cast<double>(i) / 255.0; // 0 = light, 1 = dark
        auto lower = std::lower_bound(glyphs.begin(), glyphs.end(), target, [](const GlyphDensity &a, double b) {
            return a.density < b;
        });
        if (lower == glyphs.end()) {
            lut_[i] = glyphs.back().c;
        } else {
            lut_[i] = lower->c;
        }
    }
}
