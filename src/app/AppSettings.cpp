#include "AppSettings.h"

#include "dialogs/OriConfigDlg.h"
#include "tools/OriSettings.h"

using namespace Ori::Dlg;

//------------------------------------------------------------------------------
//                            IAppSettingsListener
//------------------------------------------------------------------------------

IAppSettingsListener::IAppSettingsListener()
{
    AppSettings::instance().registerListener(this);
}

IAppSettingsListener::~IAppSettingsListener()
{
    AppSettings::instance().unregisterListener(this);
}

//------------------------------------------------------------------------------
//                                 AppSettings
//------------------------------------------------------------------------------

Q_GLOBAL_STATIC(AppSettings, __instance);

#define LOAD(option, type, default_value)\
    option = s.settings()->value(QStringLiteral(#option), default_value).to ## type()

#define SAVE(option)\
    s.settings()->setValue(QStringLiteral(#option), option)

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
    Ori::Settings s;

    s.beginGroup("Common");
    LOAD(autolimitAfterGraphGreated, Bool, true);
    LOAD(autolitmAfterAxesChanged, Bool, true);
    LOAD(highlightAxesOfSelectedGraphs, Bool, true);
    LOAD(selectNewGraph, Bool, true);
}

void AppSettings::save()
{
    Ori::Settings s;

    s.beginGroup("Common");
    SAVE(autolimitAfterGraphGreated);
    SAVE(autolitmAfterAxesChanged);
    SAVE(highlightAxesOfSelectedGraphs);
    SAVE(selectNewGraph);
}

bool AppSettings::edit()
{
    ConfigDlgOpts opts;
    opts.objectName = "AppSettingsDlg";
    opts.pageIconSize = 32;
    opts.pages = {
        ConfigPage(0, tr("General"), ":/config_pages/general"),
    };
    opts.items = {
        new ConfigItemBool(0, tr("Select just created graphs"), &selectNewGraph),
        new ConfigItemBool(0, tr("Autolimit axes after creating graphs"), &autolimitAfterGraphGreated),
        new ConfigItemBool(0, tr("Autolimit axes after choosing them for graph"), &autolitmAfterAxesChanged),
        new ConfigItemBool(0, tr("Highlight axes of selected graphs"), &highlightAxesOfSelectedGraphs),
    };
    if (ConfigDlg::edit(opts))
    {
        save();
        notify(&IAppSettingsListener::settingsChanged);
        return true;
    }
    return false;
}
