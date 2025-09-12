#pragma once
#include <QImage>
#include <QFont>
#include <vector>
#include <QString>

/**
 * Creates an Image from text
 */
class AsciiRenderer {
public:
    explicit AsciiRenderer(const QFont &font);

    QImage render(const std::vector<QString> &lines) const;

private:
    QFont font;
};
