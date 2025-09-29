#pragma once
#include <QFont>
#include <array>

#include "askier/Constants.hpp"
#undef emit

/**
 * The LUT (lookup table) is a precomputed array of 256 entries
 * where each index 0..255 (light -> dark)
 * maps the ASCII character whose measured ink density best matches the darkness level.
 *
 */
class GlyphDensityCalibrator {
public:
    explicit GlyphDensityCalibrator(const QFont &font);

    void ensureCalibrated();

    [[nodiscard]] const auto &lut() const { return lut_; }
    [[nodiscard]] double cellAspect() const { return aspect; }
    [[nodiscard]] const QFont &font() const { return font_; }
    [[nodiscard]] const auto &pixmaps() const { return pixmaps_ ;};
    [[nodiscard]] const auto &pixmapWidths() const { return pixmap_widths; }
    [[nodiscard]] const auto &pixmapHeights() const { return pixmap_heights; }

private:
    QFont font_;
    // Look up table
    std::array<char, ASCII_COUNT> lut_{};
    std::vector<unsigned char> pixmaps_{};
    std::array<int, ASCII_COUNT> pixmap_widths;
    std::array<int, ASCII_COUNT> pixmap_heights;
    double aspect = 2.0;

    void calibrate();

    bool tryLoadCache();

    void saveCache();
};
