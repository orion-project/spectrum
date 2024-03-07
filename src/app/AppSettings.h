#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include "core/OriTemplates.h"

#include <QObject>

class IAppSettingsListener
{
public:
    IAppSettingsListener();
    virtual ~IAppSettingsListener();

    virtual void settingsChanged() {}
};

class AppSettings : public QObject, public Ori::Notifier<IAppSettingsListener>
{
public:
    static AppSettings& instance();

    AppSettings();

    bool autolimitAfterGraphGreated = true;
    bool autolitmAfterAxesChanged = true;
    bool highlightAxesOfSelectedGraphs = true;
    bool selectNewGraph = true;

    bool isDevMode = false;

    void load();
    void save();
    bool edit();
};

#endif // APP_SETTINGS_H
