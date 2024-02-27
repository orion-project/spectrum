#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <QObject>

class AppSettings : public QObject
{
public:
    static AppSettings& instance();

    AppSettings();

    bool autolimitAfterGraphGreated = true;
    bool selectNewGraph = true;
    bool isDevMode = false;
    bool exportHideCursor = false;

    void load();
    void save();
};

#endif // APP_SETTINGS_H
