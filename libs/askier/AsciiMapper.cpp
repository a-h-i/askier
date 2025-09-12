#include "askier/AsciiMapper.hpp"
#include <algorithm>

AsciiMapper::AsciiMapper(const std::shared_ptr<GlyphDensityCalibrator> &calibrator) : calibrator(calibrator) {
}


char AsciiMapper::map(double luminance) const {
    // convert brightness to darkness index
    const double darkness = 1.0 - std::clamp(luminance, 0.0, 1.0);
    const auto &lut = calibrator->lut();
    int idx = static_cast<int>(std::round(darkness * 255.0));
    idx = std::clamp(idx, 0, 255);
    return lut[idx];
}
