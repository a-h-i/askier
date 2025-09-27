#include "askier/AsciiMapper.hpp"
#include "askier/Constants.hpp"

AsciiMapper::AsciiMapper(const std::shared_ptr<GlyphDensityCalibrator> &calibrator) : calibrator(calibrator) {
}


char AsciiMapper::map(double luminance) const {
    // convert brightness to darkness index
    const double darkness = 1.0 - luminance;
    const auto &lut = calibrator->lut();
    const int idx = static_cast<int>(std::round(darkness * ASCII_COUNT));
    return lut[idx];
}
