#include "askier/AsciiMapper.hpp"
#include <algorithm>

AsciiMapper::AsciiMapper(const std::shared_ptr<GlyphDensityCalibrator> &calibrator) : calibrator(calibrator) {
}


char AsciiMapper::map(double luminance) const {
    // convert brightness to darkness index
    const double darkness = 1.0 - luminance;
    const auto &lut = calibrator->lut();
    int idx = static_cast<int>(std::round(darkness * 255.0));
    return lut[idx];
}
