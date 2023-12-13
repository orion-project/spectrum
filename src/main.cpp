#include "MainWindow.h"
#include "tools/OriDebug.h"

#include <QApplication>

int main(int argc, char *argv[])
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

    //Ori::Debug::installMessageHandler();

    QApplication app(argc, argv);
    app.setApplicationName("Spectrum");
    app.setOrganizationName("orion-project.org");
    app.setStyle("Fusion");

    MainWindow w;
    w.show();

    return app.exec();
}
