#include "MainWindow.h"
#include "Operations.h"
#include "PlotWindow.h"
#include "Settings.h"
#include "core/Graph.h"
#include "helpers/OriWindows.h"
#include "helpers/OriWidgets.h"
#include "tools/OriSettings.h"
#include "widgets/DataGridPanel.h"
#include "widgets/OriMdiToolBar.h"
#include "widgets/OriStylesMenu.h"

#include <QApplication>
#include <QDebug>
#include <QDockWidget>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QStyle>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setObjectName("mainWindow");
    Ori::Wnd::setWindowIcon(this, ":/icon/main"); // TODO

    _panelDataGrid = new DataGridPanel(this);

    _operations = new Operations(this);
    connect(_operations, &Operations::graphCreated, this, &MainWindow::graphCreated);

    _mdiArea = new QMdiArea;
    connect(_mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::mdiSubWindowActivated);
    setCentralWidget(_mdiArea);

    createToolBars();
    createDocks();
    createMenu();
    createStatusBar();

    loadSettings();

    QTimer::singleShot(200, [this](){ this->newProject(); });
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
    m->addAction(tr("New Plot"), this, &MainWindow::newPlot, QKeySequence("Ctrl+N"));
    m->addSeparator();
    m->addAction(tr("Exit"), this, &MainWindow::close);

    m = menuBar()->addMenu(tr("&View"));
    Ori::Gui::makeToggleWidgetsMenu(m, tr("Panels"), {_dockDataGrid});
    Ori::Gui::makeToggleWidgetsMenu(m, tr("Toolbars"), {_toolbarMdi});
    m->addSeparator();
    m->addMenu(new Ori::Widgets::StylesMenu(this));

    m = menuBar()->addMenu(tr("&Graph"));
    m->addAction(tr("Make Random Sample"), _operations, &Operations::makeRandomSample);
    m->addAction(tr("Make From Clipboard"), _operations, &Operations::makeGraphFromClipboard);

    m = menuBar()->addMenu(tr("&Limits"));
    m->addAction(tr("Autolimits"), [this](){ auto p = this->activePlot(); if (p) p->autolimits(); }, QKeySequence("Ctrl+0"));

    m = menuBar()->addMenu(tr("&Windows"));
    m->addAction(tr("Cascade"), _mdiArea, &QMdiArea::cascadeSubWindows);
    m->addAction(tr("Tile"), _mdiArea, &QMdiArea::tileSubWindows);

}

void MainWindow::createDocks()
{
    _dockDataGrid = new QDockWidget(tr("Data Grid"));
    _dockDataGrid->setObjectName("DataGridPanel");
    _dockDataGrid->setWidget(_panelDataGrid);

    addDockWidget(Qt::LeftDockWidgetArea, _dockDataGrid);
}

void MainWindow::createStatusBar()
{
    // TODO
}

void MainWindow::createToolBars()
{
    _toolbarMdi = new Ori::Widgets::MdiToolBar(tr("Windows"), _mdiArea);
    addToolBar(Qt::BottomToolBarArea, _toolbarMdi);
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

    _operations->makeRandomSample();
}

void MainWindow::newPlot()
{
    auto plotWindow = new PlotWindow();
    auto plotTitle = QString(tr("Plot %1")).arg(_mdiArea->subWindowList().size()+1);
    plotWindow->setWindowTitle(plotTitle);
    connect(plotWindow, &PlotWindow::graphSelected, this, &MainWindow::graphSelected);

    auto mdiChild = _mdiArea->addSubWindow(plotWindow);
    mdiChild->setWindowTitle(plotWindow->windowTitle());
    mdiChild->setWindowIcon(plotWindow->windowIcon());
    mdiChild->show();
}

void MainWindow::graphCreated(Graph* graph) const
{
    auto plot = activePlot();
    if (!plot)
    {
        qWarning() << "There is no active plot window";
        // TODO: it can be no active plot when a window of another type is active.
        // It could be protocol, console, notes, etc (possible future window types).
        // We should to do something in this case: disable graph producing actions
        // or may be ask for a plot window, to place the new graph into.
        delete graph;
        return;
    }

    plot->addGraph(graph);

    if (Settings::autolimitAfterGraphGreated())
        plot->autolimits();
    if (Settings::selectNewGraph())
        plot->selectGraph(graph);
}

void MainWindow::graphSelected(Graph* graph)
{
    if (!_panelDataGrid->isVisible()) return;

    auto plot = qobject_cast<PlotWindow*>(sender());
    if (!plot) return;

    _panelDataGrid->showData(plot->plotTitle(), graph);
}

void MainWindow::mdiSubWindowActivated(QMdiSubWindow *window)
{
    if (!_panelDataGrid->isVisible()) return;

    if (!window) return;
    auto plot = qobject_cast<PlotWindow*>(window->widget());
    if (!plot) return;
    auto graph = plot->selectedGraph();
    if (!graph) return;

    _panelDataGrid->showData(plot->plotTitle(), graph);
}
