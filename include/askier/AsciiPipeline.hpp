#pragma once
#include "GlyphDensityCalibrator.hpp"
#include <vector>
#include <QImage>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#undef emit


enum DitheringType {
    None,
    FloydSteinberg,
    Ordered
};


struct AsciiParams {
    int columns;
    DitheringType dithering;
    QFont font;
};

class AsciiPipeline {
public:
    struct Result {
        std::vector<QString> lines;
        QImage preview;
        QImage midImage; // intermediate image after grayscale and gamma correction
    };

    explicit AsciiPipeline(const std::shared_ptr<GlyphDensityCalibrator> &calibrator);

    /**
     *
     * @param bgr original image in BGR format
     * @param params ASCII conversion parameters
     * @return result of the conversion
     */
    [[nodiscard]] Result process(const cv::Mat &bgr, const AsciiParams &params);

private:
    std::shared_ptr<GlyphDensityCalibrator> calibrator;
    cv::ocl::Context clContext;
    cv::UMat deviceLut, deviceDensePixmaps, devicePixmapWidths, devicePixmapHeights;
};
