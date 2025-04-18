#include "HelpSystem.h"

#include "core/OriVersion.h"
#include "helpers/OriLayouts.h"
#include "helpers/OriDialogs.h"
#include "tools/OriHelpWindow.h"
#include "widgets/OriLabels.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QFile>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStyle>
#include <QProcess>
#include <QUrl>

using namespace Ori::Layouts;

namespace {

Z::HelpSystem* __instance = nullptr;

QString homepage() { return "http://spectrum.orion-project.org"; }
QString versionFileUrl() { return "http://spectrum.orion-project.org/files/version.txt"; }
QString downloadPage() { return "http://spectrum.orion-project.org/index.php?page=dload"; }
QString sourcepage() { return "https://github.com/orion-project/spectrum"; }
QString newIssueUrl() { return "https://github.com/orion-project/spectrum/issues/new"; }
QString email() { return "spectrum@orion-project.org"; }
QString appName() { return "Spectrum"; }

} // namespace

namespace Z {

HelpSystem::HelpSystem() : QObject()
{
}

HelpSystem* HelpSystem::instance()
{
    if (!__instance)
        __instance = new HelpSystem();
    return __instance;
}

QString HelpSystem::appVersion()
{
    return QString("%1.%2.%3").arg(APP_VER_MAJOR).arg(APP_VER_MINOR).arg(APP_VER_PATCH);
}

void HelpSystem::showContent()
{
    Ori::HelpWindow::showContent();
}

void HelpSystem::showTopic(const QString& topic)
{
    Ori::HelpWindow::showTopic(topic);
}

void HelpSystem::visitHomePage()
{
    QDesktopServices::openUrl(QUrl(homepage()));
}

void HelpSystem::checkUpdates()
{
    if (_updateChecker)
    {
        qDebug() << "Check is already in progress";
        return;
    }
    _updateChecker = new QNetworkAccessManager(this);
    _updateReply = _updateChecker->get(QNetworkRequest(QUrl(versionFileUrl())));
    connect(_updateReply, &QNetworkReply::finished, this, [this](){
        if (!_updateReply) return;
        auto versionData = _updateReply->readAll();
        _updateReply->deleteLater();
        _updateReply = nullptr;
        _updateChecker->deleteLater();
        _updateChecker = nullptr;
        versionReceived(versionData);
    });
    connect(_updateReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred), this, [this](QNetworkReply::NetworkError){
        auto errorMsg =_updateReply->errorString();
        qCritical() << "Network error" << errorMsg;
        _updateReply->deleteLater();
        _updateReply = nullptr;
        _updateChecker->deleteLater();
        _updateChecker = nullptr;
        Ori::Dlg::error(tr("Failed to get version information"));
    });
}

void HelpSystem::versionReceived(QByteArray versionData) const
{
    auto versionStr = QString::fromLatin1(versionData);
    Ori::Version serverVersion(versionStr);
    Ori::Version currentVersion(APP_VER_MAJOR, APP_VER_MINOR, APP_VER_PATCH);
    if (currentVersion >= serverVersion)
        Ori::Dlg::info(tr("<p>You are using version %1"
                          "<p>Version on the server is %2"
                          "<p>You are using the most recent version of %3")
                       .arg(appVersion(), versionStr, appName()));
    else Ori::Dlg::info(tr("<p>You are using version %1"
                           "<p>Version on the server is <b>%2</b>"
                           "<p>There is a newer version of %3"
                           "<p><a href='%4'>Open download page</a>")
                        .arg(appVersion(), versionStr, appName(), downloadPage()));
}

void HelpSystem::sendBugReport()
{
    QDesktopServices::openUrl(QUrl(newIssueUrl()));
}

void HelpSystem::showAbout()
{
    auto w = new QDialog;
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setWindowTitle(tr("About %1").arg(qApp->applicationName()));

    QPixmap bckgnd(":/misc/about");
    w->setMaximumSize(bckgnd.size());
    w->setMinimumSize(bckgnd.size());
    w->resize(bckgnd.size());

    auto p = w->palette();
    p.setBrush(QPalette::Window, QBrush(bckgnd));
    w->setPalette(p);

    auto f = w->font();
#ifdef Q_OS_WIN
    f.setFamily("Trebuchet MS");
#endif
#ifdef Q_OS_MAC
    f.setFamily("Monaco"); // TODO check font look
#endif
#ifdef Q_OS_LINUX
    //f.setFamily("monospace"); system font looks well enough
#endif

    const QString textColor("color:#ddffffff");

    auto labelVersion = new QLabel(QString("%1.%2.%3").arg(APP_VER_MAJOR).arg(APP_VER_MINOR).arg(APP_VER_PATCH));
    f.setPixelSize(40);
    labelVersion->setFont(f);
    labelVersion->setStyleSheet(textColor);

    f.setPixelSize(20);
    auto labelDate = new QLabel(BUILD_DATE);
    labelDate->setFont(f);
    labelDate->setStyleSheet(textColor);

    auto labelQt = new Ori::Widgets::Label(QString("Powered by Qt %1").arg(QT_VERSION_STR));
    connect(labelQt, &Ori::Widgets::Label::clicked, []{ qApp->aboutQt(); });
    labelQt->setCursor(Qt::PointingHandCursor);
    labelQt->setStyleSheet(textColor);
    labelQt->setFont(f);

    auto makeInfo = [f, textColor](const QString& text){
        auto label = new QLabel(text);
        label->setStyleSheet(textColor);
        label->setFont(f);
        return label;
    };

    auto makeLink = [f, textColor](const QString& address, const QString& href = QString()) {
        auto label = new Ori::Widgets::Label(address);
        connect(label, &Ori::Widgets::Label::clicked, [address, href]{
            QDesktopServices::openUrl(QUrl(href.isEmpty() ? address : href));
        });
        label->setCursor(Qt::PointingHandCursor);
        label->setStyleSheet(textColor);
        label->setFont(f);
        return label;
    };

    const int horzMargin = 18;
    const int lineSpacing = 7;
    LayoutV({
        LayoutH({Stretch(), labelVersion, Space(horzMargin)}),
        LayoutH({Stretch(), Space(horzMargin)}),
        Space(lineSpacing),
        LayoutH({Stretch(), labelDate, Space(horzMargin)}),
        Stretch(),
        LayoutH({Space(horzMargin), labelQt, Stretch()}),
        Space(lineSpacing),
        LayoutH({Space(horzMargin), makeInfo(QString("Chunosov N.I. © 2005-%1").arg(APP_VER_YEAR)), Stretch()}),
        Space(lineSpacing),
        LayoutH({Space(horzMargin), makeLink(email(), QString("mailto:%1").arg(email())), Stretch()}),
        Space(lineSpacing),
        LayoutH({Space(horzMargin), makeLink(homepage()), Stretch()}),
        Space(lineSpacing),
        LayoutH({Space(horzMargin), makeLink(sourcepage()), Stretch()}),
        Space(lineSpacing),
    }).setMargin(12).setSpacing(0).useFor(w);

    w->exec();
}

} // namespace Z
