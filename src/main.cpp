#include "MainWindow.h"
#include "tools/OriDebug.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    //Ori::Debug::installMessageHandler();

    QApplication app(argc, argv);
    app.setApplicationName("Spectrum");
    app.setOrganizationName("orion-project.org");
    app.setStyle("Fusion");

    MainWindow w;
    w.show();

    return app.exec();
}
