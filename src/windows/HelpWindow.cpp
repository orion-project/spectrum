#include "HelpWindow.h"

#include "../app/AppSettings.h"
#include "../app/PersistentState.h"

#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"
#include "tools/OriHighlighter.h"

#include <QActionGroup>
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QTabBar>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QStandardPaths>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTextBrowser>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

using namespace Ori::Layouts;

namespace {

HelpWindow* __instance = nullptr;

bool isHttpUrl(const QUrl &url)
{
    return url.scheme().startsWith(QLatin1String("http"));
}

} // namespace

//------------------------------------------------------------------------------
//                               HelpBrowser
//------------------------------------------------------------------------------

class HelpBrowser : public QTextBrowser
{
public:
    HelpBrowser() : QTextBrowser()
    {
        setSearchPaths({ qApp->applicationDirPath() + "/help" });

        QFile f(":/style/help");
        if (!f.open(QIODevice::ReadOnly))
            qWarning() << "Unable to open resource file" << f.fileName() << f.errorString();
        document()->setDefaultStyleSheet(QString::fromUtf8(f.readAll()));
    }

    void setSource(const QUrl &url) override
    {
        if (isHttpUrl(url))
        {
            QDesktopServices::openUrl(url);
            return;
        }
        QTextBrowser::setSource(url);
        updateHtml();
    }

    void backward() override
    {
        QTextBrowser::backward();
        updateHtml();
    }

    void forward() override
    {
        QTextBrowser::forward();
        updateHtml();
    }

    void reload() override
    {
        QTextBrowser::reload();
        updateHtml();
    }

private:
    void updateHtml()
    {
        // setMarkdown() ignores default stylesheet, at least on Qt 5.15
        // As a workaround, we clean some markdown's styles we don't like
        // and re-set the same content as html, then the default stylesheet gets applied
        QString text = toHtml();
        static QVector<QPair<QRegularExpression, QString>> patterns = {
            { QRegularExpression("\\<h1.+?\\>"), "<h1>" },
            { QRegularExpression("\\<h2.+?\\>"), "<h2>" },
            { QRegularExpression("\\<h3.+?\\>"), "<h3>" },
        };
        for (const auto& p : patterns)
            text.replace(p.first, p.second);
        setHtml(text);
    }
};

//------------------------------------------------------------------------------
//                               HelpWindow
//------------------------------------------------------------------------------

void HelpWindow::showContent()
{
    openWindow();
    __instance->setSource("index.md");
}

void HelpWindow::showTopic(const QString& topic)
{
    openWindow();
    __instance->setSource(topic);
}

void HelpWindow::openWindow()
{
    if (!__instance)
        __instance = new HelpWindow();
    __instance->show();
    __instance->raise();
    __instance->activateWindow();
}

HelpWindow::HelpWindow() : QWidget()
{
#define T_ Ori::Gui::textToolButton

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(":/toolbar/help"));
    setWindowTitle(tr("%1 Manual").arg(qApp->applicationName()));
    setObjectName("HelpWindow");

    auto statusBar = new QStatusBar;

    _browser = new HelpBrowser;
    connect(_browser, QOverload<const QUrl&>::of(&QTextBrowser::highlighted), this, [statusBar](const QUrl& url){
        if (isHttpUrl(url))
            statusBar->showMessage(url.toString());
        else statusBar->clearMessage();
    });

    QAction *actnContent = new QAction(QIcon(":/toolbar/book"), tr("Content"), this);
    connect(actnContent, &QAction::triggered, this, [this]{ setSource("index.md"); });

    QAction *actnBack = new QAction(QIcon(":/toolbar/navigate_back"), tr("Back"), this);
    actnBack->setShortcut(QKeySequence::MoveToPreviousPage);
    QAction *actnForward = new QAction(QIcon(":/toolbar/navigate_fwd"), tr("Forward"), this);
    actnForward->setShortcut(QKeySequence::MoveToNextPage);
    connect(_browser, &QTextBrowser::backwardAvailable, actnBack, &QAction::setEnabled);
    connect(_browser, &QTextBrowser::forwardAvailable, actnForward, &QAction::setEnabled);
    connect(actnBack, &QAction::triggered, _browser, &QTextBrowser::backward);
    connect(actnForward, &QAction::triggered, _browser, &QTextBrowser::forward);

    QAction *actnRefresh = new QAction(QIcon(":/toolbar/update"), tr("Refresh"), this);
    connect(actnRefresh, &QAction::triggered, this, [this]{ _browser->reload(); });
    actnRefresh->setVisible(AppSettings::instance().isDevMode);

    QAction *actnEditStyle = new QAction(QIcon(":/toolbar/protocol"), tr("Edit Stylesheet"), this);
    connect(actnEditStyle, &QAction::triggered, this, &HelpWindow::editStyleSheet);
    actnEditStyle->setVisible(AppSettings::instance().isDevMode);

    auto toolbar = new QToolBar;
    Ori::Gui::populate(toolbar, { T_(actnContent), nullptr, T_(actnBack), T_(actnForward), nullptr, actnRefresh, actnEditStyle });

    LayoutV({toolbar, _browser, statusBar}).setSpacing(0).setMargins(3, 0, 3, 0).useFor(this);

    PersistentState::restoreWindowGeometry("help", this, {800, 600});
}

HelpWindow::~HelpWindow()
{
    PersistentState::storeWindowGeometry("help", this);

    __instance = nullptr;
}

void HelpWindow::setSource(const QString& name)
{
    static QRegularExpression hasExt("^.+\\.{1}.+$");
    if (hasExt.match(name).hasMatch())
        _browser->setSource(QUrl(name));
    else
        _browser->setSource(QUrl(name + ".md"));
}

void HelpWindow::editStyleSheet()
{
    QString styleFile = qApp->applicationDirPath() + "/../src/help.css";

    auto editor = new QPlainTextEdit;
    Ori::Gui::setFontMonospace(editor);
    editor->setPlainText(_browser->document()->defaultStyleSheet());
    Ori::Highlighter::setHighlighter(editor, ":/syntax/css");

    auto html = new QPlainTextEdit;
    Ori::Gui::setFontMonospace(html);
    html->setPlainText(_browser->toHtml());

    auto tabs = new QTabWidget;
    tabs->addTab(editor, "CSS");
    tabs->addTab(html, "HTML");

    auto applyButton = new QPushButton("Apply");
    connect(applyButton, &QPushButton::clicked, this, [this, editor]{
        _browser->document()->setDefaultStyleSheet(editor->toPlainText());
    });

    auto reloadButton = new QPushButton("Reload");
    connect(reloadButton, &QPushButton::clicked, this, [this, editor, html]{
        editor->setPlainText(_browser->document()->defaultStyleSheet());
        html->setPlainText(_browser->toHtml());
    });

    auto saveButton = new QPushButton("Save");
    saveButton->connect(saveButton, &QPushButton::clicked, editor, [styleFile, editor]{
        QFile f(styleFile);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qWarning() << "Unable to open file for writing" << styleFile << f.errorString();
            return;
        }
        f.write(editor->toPlainText().toUtf8());
        qDebug() << "Saved" << styleFile;
    });

    auto wnd = Ori::Layouts::LayoutV({
        new QLabel(styleFile),
        tabs,
        Ori::Layouts::LayoutH({
            Ori::Layouts::Stretch(),
            reloadButton,
            applyButton,
            saveButton,
        }).setMargin(6)
    }).setMargin(3).setSpacing(6).makeWidget();
    wnd->setAttribute(Qt::WA_DeleteOnClose);
    wnd->setWindowTitle("Stylesheet Editor");
    wnd->setWindowIcon(QIcon(":/toolbar/protocol"));
    wnd->resize(300, 600);
    wnd->show();
}
