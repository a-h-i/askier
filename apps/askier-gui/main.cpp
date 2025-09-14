#include <QApplication>
#include "gui/MainWindow.hpp"
#include "askier/version.hpp"
#include <string>
#include <algorithm>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    const std::string appname = ASKIER_NAME;
    std::string appname_lower;
    std::transform(appname.begin(), appname.end(), appname_lower.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    app.setApplicationName(appname_lower.c_str());
    app.setApplicationVersion(ASKIER_VERSION);

    MainWindow window;
    window.show();
    return app.exec();
}