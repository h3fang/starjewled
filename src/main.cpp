#include <QApplication>
#include <QCommandLineParser>

#include "overlay.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOptions({
        {{"a", "automation"}, "enable automation"},
        {{"i", "interval"}, "time interval between automated actions", "milliseconds", "1000"},
    });
    parser.process(app);

    Overlay overlay(parser.isSet("a"), parser.value("i").toInt());
    overlay.showMaximized();

    return app.exec();
}
