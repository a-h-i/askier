#pragma once

#include <memory>
#include "GlyphDensityCalibrator.hpp"

/**
 * Maps luminance value to closest ascii character
 */
class AsciiMapper {
public:
    explicit AsciiMapper(const std::shared_ptr<GlyphDensityCalibrator> &calibrator);

    /**
     *
     * @param luminance 0 = dark, 1.0 = bright
     * @return mapped glyph from GlyphDensityCalibrator
     */
    char map(double luminance) const;

private:
    std::shared_ptr<GlyphDensityCalibrator> calibrator;
};