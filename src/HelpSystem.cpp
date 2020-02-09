#include "HelpSystem.h"

#include "core/OriVersion.h"
#include "helpers/OriLayouts.h"
#include "helpers/OriDialogs.h"
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
    return QString("%1.%2.%3-%4").arg(APP_VER_MAJOR).arg(APP_VER_MINOR).arg(APP_VER_PATCH).arg(APP_VER_CODENAME);
}

void HelpSystem::showContents()
{
    if (!startAssistant()) return;

    QByteArray commands;
    commands.append("show contents;");
    commands.append("expandToc 3;");
    commands.append("setSource qthelp://org.orion-project.spectrum/doc/index.html\n");
    _assistant->write(commands);
}

void HelpSystem::showIndex()
{
    if (!startAssistant()) return;

    QByteArray commands;
    commands.append("show index\n");
    _assistant->write(commands);
}

void HelpSystem::showTopic(const QString& topic)
{
    if (!startAssistant()) return;

    QByteArray commands;
    commands.append("setSource qthelp://org.orion-project.spectrum/doc/" % topic % '\n');
    _assistant->write(commands);
}

bool HelpSystem::startAssistant()
{
    if (_assistant)
    {
        if (_assistant->state() == QProcess::Running) return true;

        delete _assistant;
        _assistant = nullptr;
    }

    QString appDir = qApp->applicationDirPath();
    QString helpFile = appDir + "/spectrum.qhc";
#ifdef Q_OS_WIN
    QString assistantFile = appDir + "/assistant.exe";
#else
    QString assistantFile = appDir + "/assistant";
#endif

    if (!QFile::exists(assistantFile))
    {
        Ori::Dlg::error("Help viewer not found");
        return false;
    }

    if (!QFile::exists(helpFile))
    {
        Ori::Dlg::error("Help file not found");
        return false;
    }

    QProcess *process = new QProcess(this);
    process->start(assistantFile, {
                       "-collectionFile", helpFile,
                       "-style", qApp->style()->objectName(),
                       "-enableRemoteControl"
                   });
    if (!process->waitForStarted(5000))
    {
        Ori::Dlg::error("Failed to start help viewer: " + process->errorString());
        delete process;
        return false;
    }
    connect(process, SIGNAL(finished(int)), this, SLOT(assistantFinished(int)));
    connect(process, &QProcess::readyReadStandardOutput, [this](){
        qDebug() << QString::fromLocal8Bit(_assistant->readAllStandardOutput());
    });
    connect(process, &QProcess::readyReadStandardError, [this](){
        qDebug() << QString::fromLocal8Bit(_assistant->readAllStandardError());
    });
    connect(qApp, &QApplication::aboutToQuit, this, &HelpSystem::closeAssistant);
    _assistant = process;
    return true;
}

void HelpSystem::assistantFinished(int exitCode)
{
    qDebug() << "Help viewer finished, exit code" << exitCode;
    if (_assistant)
    {
        _assistant->deleteLater();
        _assistant = nullptr;
    }
}

void HelpSystem::closeAssistant()
{
    if (!_assistant) return;
    if (_assistant->state() == QProcess::Running)
    {
        _assistant->terminate();
        _assistant->waitForFinished(5000);
    }
    delete _assistant;
    _assistant = nullptr;
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
    connect(_updateReply, &QNetworkReply::finished, [this](){
        if (!_updateReply) return;
        auto versionData = _updateReply->readAll();
        _updateReply->deleteLater();
        _updateReply = nullptr;
        _updateChecker->deleteLater();
        _updateChecker = nullptr;
        versionReceived(versionData);
    });
    connect(_updateReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), [this](QNetworkReply::NetworkError){
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

    QPixmap bckgnd(":/style/about"); // TODO
    w->setMaximumSize(bckgnd.size());
    w->setMinimumSize(bckgnd.size());
    w->resize(bckgnd.size());

    auto p = w->palette();
    p.setBrush(QPalette::Background, QBrush(bckgnd));
    w->setPalette(p);

    auto f = w->font();
#ifdef Q_OS_WIN
    f.setFamily("Consolas");
#endif
#ifdef Q_OS_MAC
    f.setFamily("Monaco"); // Menlo?
#endif
#ifdef Q_OS_LINUX
    f.setFamily("monospace");
#endif

    auto labelVersion = new QLabel(QString("%1.%2.%3").arg(APP_VER_MAJOR).arg(APP_VER_MINOR).arg(APP_VER_PATCH));
    f.setPixelSize(40);
    f.setBold(true);
    labelVersion->setFont(f);
    labelVersion->setStyleSheet("color:white");

    f.setPixelSize(32);
    auto labelCodename = new QLabel(APP_VER_CODENAME);
    labelCodename->setFont(f);
    labelCodename->setStyleSheet("color:white");

    f.setBold(false);
    f.setPixelSize(18);
    auto labelDate = new QLabel(BUILDDATE);
    labelDate->setFont(f);
    labelDate->setStyleSheet("color:white");

    auto labelQt = new Ori::Widgets::Label(QString("Powered by Qt %1").arg(QT_VERSION_STR));
    connect(labelQt, &Ori::Widgets::Label::clicked, []{ qApp->aboutQt(); });
    labelQt->setCursor(Qt::PointingHandCursor);
    labelQt->setStyleSheet("color:white");
    labelQt->setFont(f);

    auto makeInfo = [f](const QString& text){
        auto label = new QLabel(text);
        label->setStyleSheet("color:white");
        label->setFont(f);
        return label;
    };

    auto makeLink = [f](const QString& address, const QString& href = QString()) {
        auto label = new Ori::Widgets::Label(address);
        connect(label, &Ori::Widgets::Label::clicked, [address, href]{
            QDesktopServices::openUrl(QUrl(href.isEmpty() ? address : href));
        });
        label->setCursor(Qt::PointingHandCursor);
        label->setStyleSheet("color:white");
        label->setFont(f);
        return label;
    };

    f.setPixelSize(9);
    auto labelDescr = new QLabel(
        "The program is provided as is with no warranty of any kind, "
        "including the warranty of design, merchantability and fitness for a particular purpose.");
    labelDescr->setWordWrap(true);
    labelDescr->setStyleSheet("color:#88FFFFFF");
    labelDescr->setFont(f);

    LayoutV({
        LayoutH({Stretch(), labelVersion, Space(4)}),
        LayoutH({Stretch(), labelCodename, Space(4)}),
        Space(4),
        LayoutH({Stretch(), labelDate, Space(4)}),
        Stretch(),
        LayoutH({Space(4), labelQt, Stretch()}),
        Space(4),
        LayoutH({Space(4), makeInfo(QString("Chunosov N.I. © 2005-%1").arg(APP_VER_YEAR)), Stretch()}),
        Space(4),
        LayoutH({Space(4), makeLink(email(), QString("mailto:%1").arg(email())), Stretch()}),
        Space(4),
        LayoutH({Space(4), makeLink(homepage()), Stretch()}),
        Space(4),
        LayoutH({Space(4), makeLink(sourcepage()), Stretch()}),
        Space(4),
    }).setMargin(12).setSpacing(0).useFor(w);

    w->exec();
}

} // namespace Z
