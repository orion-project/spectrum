#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <QObject>

class AppSettings : public QObject
{
public:
    static AppSettings& instance();

     AppSettings();

    static bool autolimitAfterGraphGreated();
    static bool selectNewGraph();

    bool isDevMode = false;

    void load();
    void save();
};

#endif // APP_SETTINGS_H
