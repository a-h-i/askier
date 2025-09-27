#include "askier/AsciiMapper.hpp"

AsciiMapper::AsciiMapper(const std::shared_ptr<GlyphDensityCalibrator> &calibrator) : calibrator(calibrator) {
}


char AsciiMapper::map(double luminance) const {
    // convert brightness to darkness index
    const double darkness = 1.0 - luminance;
    const auto &lut = calibrator->lut();
    const auto lutSize = lut.size();
    const int maxIndex = lutSize - 1;
    const auto idx = static_cast<int>(std::round(darkness * maxIndex));
    return lut[std::clamp(0, idx, maxIndex)];
}
