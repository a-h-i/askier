#pragma once
#include <QSlider>


class DoubleSlider : public QSlider {
    Q_OBJECT

public:
    explicit DoubleSlider(Qt::Orientation orientation, QWidget *parent = nullptr);

    /**
     * Control mapping precisions i.e., how many decimal places preserved
     * @param decimals
     */
    void setDecimals(int decimals);

    int decimals() const;

    void setRange(double min, double max);

    void setMinimum(double min);

    void setMaximum(double max);

    double minimum() const;

    double maximum() const;

    void setSingleStep(double step);

    void setTickInterval(double interval);

    void setValue(double value);

    double value() const;

    signals:
    

    void valueChanged(double value);

private
    slots:
    

    void handleIntValueChanged(int value);

private:
    int scaledSpan() const;

    int toInt(double v) const;

    int toIntStep(double step) const;

    double toDouble(int v) const;

    double min, max, decimals_, scale;
};