#include "gui/ConversionParamsDialog.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>


static const std::string NONE_DITHERING = "None";
static const std::string FLOYD_STEINBERG_DITHERING = "Floyd-Steinberg";
static const std::string ATKINSON_DITHERING = "Ordered";


ConversionParamsDialog::ConversionParamsDialog(AsciiParams currentParams, QWidget *parent) : QDialog(parent),
    params(currentParams) {
    label = new QLabel("Adjust Conversion Parameters", this);

    dithering_combo = new QComboBox(this);
    dithering_combo->addItem(NONE_DITHERING.c_str());
    dithering_combo->addItem(FLOYD_STEINBERG_DITHERING.c_str());
    dithering_combo->addItem(ATKINSON_DITHERING.c_str());

    if (currentParams.dithering == DitheringType::FloydSteinberg) {
        dithering_combo->setCurrentIndex(1);
    } else if (currentParams.dithering == DitheringType::Ordered) {
        dithering_combo->setCurrentIndex(2);
    } else {
        dithering_combo->setCurrentIndex(0);
    }
    dithering_combo->setInsertPolicy(QComboBox::NoInsert);
    dithering_combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(dithering_combo, &QComboBox::currentTextChanged, this, &ConversionParamsDialog::onDitheringChanged);

    gamma_slider = new DoubleSlider(Qt::Horizontal, this);
    gamma_slider->setDecimals(3);
    gamma_slider->setMinimum(0.1);
    gamma_slider->setMaximum(20.0);
    gamma_slider->setValue(currentParams.gamma);
    connect(gamma_slider, &DoubleSlider::valueChanged, this, &ConversionParamsDialog::onGammaChanged);

    apply_button = new QPushButton("Apply", this);
    connect(apply_button, &QPushButton::clicked, this, &ConversionParamsDialog::accept);
    cancel_button = new QPushButton("Cancel", this);
    connect(cancel_button, &QPushButton::clicked, this, &ConversionParamsDialog::reject);
    QLabel *gammaLabel = new QLabel("Gamma correction", this);
    gammaLabel->setAlignment(Qt::AlignCenter);
    QLabel *ditheringLabel = new QLabel("Dithering", this);
    ditheringLabel->setAlignment(Qt::AlignCenter);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(label);
    layout->addSpacing(10);
    layout->addWidget(ditheringLabel);
    layout->addWidget(dithering_combo);
    layout->addSpacing(5);

    QHBoxLayout *gamma_slider_layout = new QHBoxLayout();
    gamma_slider_layout->setContentsMargins(0, 0, 0, 0);
    gamma_slider_layout->setSpacing(6);
    QLabel *gamma_min_label = new QLabel(QString::number(gamma_slider->minimum()), this);
    QLabel *gamma_max_label = new QLabel(QString::number(gamma_slider->maximum()), this);

    gamma_slider_layout->addWidget(gamma_min_label);
    gamma_slider_layout->addWidget(gamma_slider);
    gamma_slider_layout->addWidget(gamma_max_label);


    QVBoxLayout *gamma_layout = new QVBoxLayout();
    QLabel *gamma_value_label = new QLabel(QString::number(gamma_slider->value()), this);
    gamma_value_label->setAlignment(Qt::AlignCenter);
    gamma_layout->addWidget(gammaLabel);
    gamma_layout->addLayout(gamma_slider_layout);
    gamma_layout->addWidget(gamma_value_label);

    connect(gamma_slider, &DoubleSlider::valueChanged, gamma_value_label, [gamma_value_label](double value) {
        gamma_value_label->setText(QString::number(value));
    });

    layout->addLayout(gamma_layout);

    QVBoxLayout *sampling_layout = new QVBoxLayout();
    QLabel *sampling_label = new QLabel("Sampling", this);
    QLabel *sampling_value_label = new QLabel(QString::number(params.supersampling_scale), this);
    sampling_value_label->setAlignment(Qt::AlignCenter);
    sampling_slider = new QSlider(Qt::Horizontal, this);
    sampling_slider->setRange(1, 10);
    sampling_slider->setValue(params.supersampling_scale);
    connect(sampling_slider, &QSlider::valueChanged, sampling_value_label, [sampling_value_label](int value) {
        sampling_value_label->setText(QString::number(value));
    });
    connect(sampling_slider, &QSlider::valueChanged, this, &ConversionParamsDialog::onSamplingChanged);
    QLabel *sampling_min_label = new QLabel("1", this);
    QLabel *sampling_max_label = new QLabel("10", this);

    sampling_layout->addWidget(sampling_label);
    QHBoxLayout *sampling_slider_layout = new QHBoxLayout();
    sampling_slider_layout->addWidget(sampling_min_label);
    sampling_slider_layout->addWidget(sampling_slider);
    sampling_slider_layout->addWidget(sampling_max_label);
    sampling_layout->addLayout(sampling_slider_layout);
    sampling_layout->addWidget(sampling_value_label);

    layout->addLayout(sampling_layout);

    QHBoxLayout *buttons_layout = new QHBoxLayout();
    buttons_layout->addWidget(apply_button);
    buttons_layout->addWidget(cancel_button);
    layout->addLayout(buttons_layout);

    setLayout(layout);
}

void ConversionParamsDialog::onSamplingChanged(int value) {
    params.supersampling_scale = value;
}

AsciiParams ConversionParamsDialog::getParams() const {
    return params;
}

void ConversionParamsDialog::onDitheringChanged(const QString &text) {
    if (text == NONE_DITHERING) {
        params.dithering = None;
    } else if (text == FLOYD_STEINBERG_DITHERING) {
        params.dithering = FloydSteinberg;
    } else if (text == ATKINSON_DITHERING) {
        params.dithering = Ordered;
    }
}

void ConversionParamsDialog::onGammaChanged(double v) {
    params.gamma = v;
}
