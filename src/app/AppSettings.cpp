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
    LOAD(lockPanZoomToSelectedGraphs, Bool, true);
}

void AppSettings::save()
{
    Ori::Settings s;

    s.beginGroup("Common");
    SAVE(autolimitAfterGraphGreated);
    SAVE(autolitmAfterAxesChanged);
    SAVE(highlightAxesOfSelectedGraphs);
    SAVE(selectNewGraph);
    SAVE(lockPanZoomToSelectedGraphs);
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
        new ConfigItemSpace(0, 12),
        (new ConfigItemSection(0, tr("Multi-axis")))
            ->withHint(tr("The next options make sense only if there are several axes of the same orientation")),
        new ConfigItemBool(0, tr("Autolimit axes after they was assigned to graph"), &autolitmAfterAxesChanged),
        new ConfigItemBool(0, tr("Highlight axes of selected graphs"), &highlightAxesOfSelectedGraphs),
        new ConfigItemBool(0, tr("Use only selected graphs' axes for pan and zoom"), &lockPanZoomToSelectedGraphs),
    };
    if (ConfigDlg::edit(opts))
    {
        save();
        notify(&IAppSettingsListener::settingsChanged);
        return true;
    }
    return false;
}
