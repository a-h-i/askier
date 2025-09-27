#pragma once
#include <QFont>
#include <array>

#include "askier/Constants.hpp"

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

    const auto &lut() const { return lut_; }
    double cellAspect() const { return aspect; }
    const QFont &font() const { return font_; }

private:
    QFont font_;
    // Look up table
    std::array<char, ASCII_COUNT> lut_{};
    double aspect = 2.0;

    void calibrate();

    bool tryLoadCache();

    void saveCache();
};