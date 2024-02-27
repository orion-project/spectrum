#include "AppSettings.h"

Q_GLOBAL_STATIC(AppSettings, __instance);

AppSettings& AppSettings::instance()
{
    return *__instance;
}

AppSettings::AppSettings() : QObject()
{
    load();
}

void AppSettings::load()
{
    // TODO
}

void AppSettings::save()
{
    // TODO
}
