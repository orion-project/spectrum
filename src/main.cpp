#include "app/AppSettings.h"
#include "app/HelpSystem.h"
#include "tests/TestSuite.h"
#include "windows/MainWindow.h"

#include "helpers/OriTheme.h"
#include "testing/OriTestManager.h"
#include "tools/OriHelpWindow.h"
#include "tools/OriDebug.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>

#ifndef Q_OS_WIN
#include <iostream>
#endif

int main(int argc, char *argv[])
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

    QApplication app(argc, argv);
    app.setApplicationName("Spectrum");
    app.setApplicationDisplayName("Spectrum");
    app.setOrganizationName("orion-project.org");
    app.setApplicationVersion(Z::HelpSystem::appVersion());
    app.setStyle("Fusion");

    QCommandLineParser parser;
    auto optionHelp = parser.addHelpOption();
    auto optionVersion = parser.addVersionOption();
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    QCommandLineOption optionTest("test", "Run unit-test session.");
    QCommandLineOption optionDevMode("dev"); optionDevMode.setFlags(QCommandLineOption::HiddenFromHelp);
    QCommandLineOption optionConsole("console"); optionConsole.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOptions({optionTest, optionDevMode, optionConsole});

    if (!parser.parse(QApplication::arguments()))
    {
#ifdef Q_OS_WIN
        QMessageBox::critical(nullptr, app.applicationName(), parser.errorText());
#else
        std::cerr << qPrintable(parser.errorText()) << std::endl;
#endif
        return 1;
    }

    // These will quite the app
    if (parser.isSet(optionHelp))
        parser.showHelp();
    if (parser.isSet(optionVersion))
        parser.showVersion();

    // It's only useful on Windows where there is no
    // direct way to use the console for GUI applications.
    if (parser.isSet(optionConsole))
        Ori::Debug::installMessageHandler();

    // Run test session if requested
    if (parser.isSet(optionTest))
        return Ori::Testing::run(app, { ADD_SUITE(Z::Tests) });

    // Load application settings before any command start
    AppSettings::instance().isDevMode = parser.isSet(optionDevMode);
    Ori::HelpWindow::isDevMode = AppSettings::instance().isDevMode;

    // Call `setStyleSheet` after setting loaded
    // to be able to apply custom colors.
    app.setStyleSheet(Ori::Theme::makeStyleSheet(Ori::Theme::loadRawStyleSheet()));

    MainWindow w;
    w.show();

    return app.exec();
}
