#pragma once
#include "GlyphDensityCalibrator.hpp"
#include <vector>
#include <QImage>
#include <opencv2/core.hpp>


enum DitheringType {
    None,
    FloydSteinberg,
    Ordered
};


struct AsciiParams {
    int columns;
    /**
     * Apply an adjustable gamma curve to the luminance values before mapping them to glyphs
     * gamma > 1 brightens mid-tones
     * gamma < 1 darkens mid-tones
     * gamma = 1 keeps luminance unchanged
     */
    double gamma;
    DitheringType dithering;
    QFont font;
    int supersampling_scale;
};

class AsciiPipeline {
public:
    struct Result {
        std::vector<QString> lines;
        QImage preview;
    };

    explicit AsciiPipeline(const std::shared_ptr<GlyphDensityCalibrator> &calibrator);

    /**
     *
     * @param bgr original image in BGR format
     * @param params ASCII conversion parameters
     * @return result of the conversion
     */
    [[nodiscard]] Result process(const cv::Mat &bgr, const AsciiParams &params) const;

private:
    std::shared_ptr<GlyphDensityCalibrator> calibrator;
};