#include "gui/ConversionParamsDialog.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>


static const std::string NONE_DITHERING = "None";
static const std::string FLOYD_STEINBERG_DITHERING = "Floyd-Steinberg";
static const std::string ATKINSON_DITHERING = "Ordered";


ConversionParamsDialog::ConversionParamsDialog(const AsciiParams &currentParams, QWidget *parent) : QDialog(parent),
    params(currentParams) {
    label = new QLabel("Adjust Conversion Parameters", this);

    dithering_combo = new QComboBox(this);
    dithering_combo->addItem(NONE_DITHERING.c_str());
    dithering_combo->addItem(FLOYD_STEINBERG_DITHERING.c_str());
    dithering_combo->addItem(ATKINSON_DITHERING.c_str());

    if (params.dithering == DitheringType::FloydSteinberg) {
        dithering_combo->setCurrentIndex(1);
    } else if (params.dithering == DitheringType::Ordered) {
        dithering_combo->setCurrentIndex(2);
    } else {
        dithering_combo->setCurrentIndex(0);
    }
    dithering_combo->setInsertPolicy(QComboBox::NoInsert);
    dithering_combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(dithering_combo, &QComboBox::currentTextChanged, this, &ConversionParamsDialog::onDitheringChanged);

    columns_slider = new QSlider(Qt::Horizontal, this);
    columns_slider->setRange(100, 1080);
    columns_slider->setValue(params.columns);

    auto colSlidersInnerLayout = new QHBoxLayout();
    colSlidersInnerLayout->addWidget(new QLabel(QString::number(columns_slider->minimum())));
    colSlidersInnerLayout->addWidget(columns_slider);
    colSlidersInnerLayout->addWidget(new QLabel(QString::number(columns_slider->maximum())));

    auto colsSliderOuterLayout = new QVBoxLayout();
    colsSliderOuterLayout->addLayout(colSlidersInnerLayout);
    colsSliderOuterLayout->addSpacing(5);
    auto colsValue = new QLabel(QString::number(columns_slider->value()));
    colsValue->setAlignment(Qt::AlignCenter);
    connect(columns_slider, &QSlider::valueChanged, this, [colsValue, this](int value) {
        colsValue->setText(QString::number(value));
        this->params.columns = value;
    });
    colsSliderOuterLayout->addWidget(colsValue);

    apply_button = new QPushButton("Apply", this);
    connect(apply_button, &QPushButton::clicked, this, &ConversionParamsDialog::accept);
    cancel_button = new QPushButton("Cancel", this);
    connect(cancel_button, &QPushButton::clicked, this, &ConversionParamsDialog::reject);
    QLabel *ditheringLabel = new QLabel("Dithering", this);
    ditheringLabel->setAlignment(Qt::AlignCenter);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(label);
    layout->addSpacing(10);
    layout->addWidget(ditheringLabel);
    layout->addWidget(dithering_combo);
    layout->addSpacing(5);
    layout->addLayout(colsSliderOuterLayout);
    layout->addSpacing(5);
    QHBoxLayout *buttons_layout = new QHBoxLayout();
    buttons_layout->addWidget(apply_button);
    buttons_layout->addWidget(cancel_button);
    layout->addLayout(buttons_layout);

    setLayout(layout);
}


AsciiParams ConversionParamsDialog::getParams() const {
    return params;
}

void ConversionParamsDialog::onDitheringChanged(const QString &text) {
    if (text == NONE_DITHERING.c_str()) {
        params.dithering = None;
    } else if (text == FLOYD_STEINBERG_DITHERING.c_str()) {
        params.dithering = FloydSteinberg;
    } else if (text == ATKINSON_DITHERING.c_str()) {
        params.dithering = Ordered;
    }
}