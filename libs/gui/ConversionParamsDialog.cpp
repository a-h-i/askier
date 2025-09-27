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


