#pragma once
#include <QDialog>
#include <QLabel>
#include <QMenuBar>
#include <QComboBox>
#include <QPushButton>

#include "askier/AsciiPipeline.hpp"
#include "gui/DoubleSlider.hpp"

class ConversionParamsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConversionParamsDialog(AsciiParams currentParams, QWidget *parent = nullptr);

    AsciiParams getParams() const;

public
    slots:
    
    void onSamplingChanged(int value);
    void onDitheringChanged(const QString &text);

    void onGammaChanged(double value);

private:
    QLabel *label;
    QComboBox *dithering_combo;
    DoubleSlider *gamma_slider;
    QSlider *sampling_slider;
    QPushButton *apply_button, *cancel_button;
    AsciiParams params;
};