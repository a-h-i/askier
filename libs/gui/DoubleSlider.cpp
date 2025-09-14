#include "gui/DoubleSlider.hpp"


DoubleSlider::DoubleSlider(Qt::Orientation orientation, QWidget *parent) : QSlider(orientation, parent),
                                                                           min(0.0), max(1.0), decimals_(3),
                                                                           scale(std::pow(10.0, decimals_)) {
    QSlider::setRange(min, scaledSpan());
    connect(this, &QSlider::valueChanged, this, &DoubleSlider::handleIntValueChanged);
}


void DoubleSlider::setDecimals(int decimals) {
    if (decimals < 0) {
        decimals = 0;
    }
    if (decimals == decimals_) {
        return;
    }
    decimals_ = decimals;
    scale = std::pow(10.0, decimals_);
    const double cur = value();
    QSlider::setRange(min, scaledSpan());
    setValue(cur);
}

int DoubleSlider::decimals() const {
    return decimals_;
}

void DoubleSlider::setRange(double min, double max) {
    if (min > max) {
        std::swap(min, max);
    }
    this->min = min;
    this->max = max;
    QSlider::setRange(min, scaledSpan());
    setValue(value());
}

void DoubleSlider::setMinimum(double min) {
    setRange(min, max);
}

void DoubleSlider::setMaximum(double max) {
    setRange(min, max);
}

double DoubleSlider::minimum() const {
    return min;
}

double DoubleSlider::maximum() const {
    return max;
}

void DoubleSlider::setSingleStep(double step) {
    if (step <= 0.0) {
        return;
    }
    QSlider::setPageStep(toIntStep(step));
}

void DoubleSlider::setTickInterval(double interval) {
    if (interval <= 0.0) {
        return;
    }
    QSlider::setTickInterval(toIntStep(interval));
}

void DoubleSlider::setValue(double v) {
    QSlider::setValue(toInt(v));
}

double DoubleSlider::value() const {
    return toDouble(QSlider::value());
}

int DoubleSlider::scaledSpan() const {
    if (max <= min) {
        return 0;
    }
    return std::max(0, static_cast<int>(std::round(scale * (max - min))));
}

void DoubleSlider::handleIntValueChanged(int v) {
    emit valueChanged(toDouble(v));
}

int DoubleSlider::toInt(double v) const {
    if (v <= min) {
        return 0;
    }
    if (v >= max) {
        return scaledSpan();
    }
    return static_cast<int>(std::round((v - min) * scale));
}

int DoubleSlider::toIntStep(double step) const {
    // Convert a double step to int step, minimum 1
    const auto s = static_cast<int>(std::llround(step * scale));
    return std::max(1, s);
}

double DoubleSlider::toDouble(int v) const {
    if (v <= 0) {
        return min;
    }
    const int span = scaledSpan();
    if (v >= span) {
        return max;
    }
    return min + static_cast<double>(v) / scale;
}