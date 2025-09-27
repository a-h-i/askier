#pragma once
#include <QDialog>
#include <QLabel>
#include <QMenuBar>
#include <QComboBox>
#include <QPushButton>

#include "askier/AsciiPipeline.hpp"

class ConversionParamsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConversionParamsDialog(const AsciiParams &currentParams, QWidget *parent = nullptr);

    AsciiParams getParams() const;

public
    slots:
    
    void onDitheringChanged(const QString &text);


private:
    QLabel *label;
    QComboBox *dithering_combo;
    QPushButton *apply_button, *cancel_button;
    AsciiParams params;
};