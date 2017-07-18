#include "MainWindow.h"
#include "Operations.h"
#include "PlotWindow.h"
#include "core/Graph.h"
#include "helpers/OriWindows.h"
#include "tools/OriSettings.h"
#include "widgets/OriMdiToolBar.h"
#include "widgets/OriStylesMenu.h"

#include <QApplication>
#include <QDebug>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setObjectName("mainWindow");
    Ori::Wnd::setWindowIcon(this, ":/icon/main"); // TODO

    _operations = new Operations(this);
    connect(_operations, &Operations::graphCreated, this, &MainWindow::graphCreated);

    _mdiArea = new QMdiArea;
    setCentralWidget(_mdiArea);

    createMenu();
    createDocks();
    createToolBars();
    createStatusBar();

    loadSettings();

    newProject();
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::saveSettings()
{
    Ori::Settings s;
    s.storeWindowGeometry(this);
    s.storeDockState(this);
    s.setValue("style", qApp->style()->objectName());
}

void MainWindow::loadSettings()
{
    Ori::Settings s;
    s.restoreWindowGeometry(this);
    s.restoreDockState(this);
    qApp->setStyle(s.strValue("style"));
}

void MainWindow::createMenu()
{
    QMenu *m;

    m = menuBar()->addMenu(tr("&Project"));
    m->addAction(tr("New Plot"), this, &MainWindow::newPlot);
    m->addSeparator();
    m->addAction(tr("Exit"), this, &MainWindow::close);

    m = menuBar()->addMenu(tr("&Graph"));
    m->addAction(tr("Make Random Sample"), _operations, &Operations::makeRandomSample);
    m->addAction(tr("Make From Clipboard"), _operations, &Operations::makeGraphFromClipboard);

    m = menuBar()->addMenu(tr("&View"));
    m->addMenu(new Ori::Widgets::StylesMenu(this));

    m = menuBar()->addMenu(tr("&Limits"));
    m->addAction(tr("Autolimits"), [this](){ auto p = this->activePlot(); if (p) p->autolimits(); }, QKeySequence("Ctrl+0"));

    m = menuBar()->addMenu(tr("&Windows"));
    m->addAction(tr("Cascade"), _mdiArea, &QMdiArea::cascadeSubWindows);
    m->addAction(tr("Tile"), _mdiArea, &QMdiArea::tileSubWindows);

}

void MainWindow::createDocks()
{
    // TDOD
}

void MainWindow::createStatusBar()
{
    // TODO
}

void MainWindow::createToolBars()
{
    addToolBar(Qt::BottomToolBarArea, new Ori::Widgets::MdiToolBar(tr("Windows"), _mdiArea));
}

PlotWindow* MainWindow::activePlot() const
{
    auto mdiChild = _mdiArea->currentSubWindow();
    if (!mdiChild) return nullptr;
    return dynamic_cast<PlotWindow*>(mdiChild->widget());
}

void MainWindow::newProject()
{
    newPlot();
}

void MainWindow::newPlot()
{
    auto plotWindow = new PlotWindow();
    auto mdiChild = _mdiArea->addSubWindow(plotWindow);
    auto title = plotWindow->windowTitle();
    if (title.isEmpty())
        title = QString(tr("Plot %1").arg(_mdiArea->subWindowList().size()));
    mdiChild->setWindowTitle(title);
    mdiChild->setWindowIcon(plotWindow->windowIcon());
    mdiChild->show();
}

void MainWindow::graphCreated(Graph* g) const
{
    auto plot = activePlot();

    if (!plot)
    {
        qWarning() << "There is no active plot window";
        // TODO: it can be no active plot when a window of another type is active.
        // It could be protocol, console, notes, etc (possible future window types).
        // We should to do something in this case: disable graph producing actions
        // or may be ask for a plot window, to place the new graph into.
        delete g;
        return;
    }

    plot->addGraph(g);
}
