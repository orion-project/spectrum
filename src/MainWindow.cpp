#include "MainWindow.h"

#include "HelpSystem.h"
#include "Operations.h"
#include "PlotWindow.h"
#include "Settings.h"
#include "core/Graph.h"
#include "widgets/DataGridPanel.h"

#include "helpers/OriWindows.h"
#include "helpers/OriWidgets.h"
#include "tools/OriSettings.h"
#include "widgets/OriMdiToolBar.h"
#include "widgets/OriStatusBar.h"

#include <QApplication>
#include <QDebug>
#include <QDockWidget>
#include <QLabel>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QStyle>
#include <QTimer>

enum StatusPanels
{
    STATUS_GRAPHS,
    STATUS_MODIF,
    STATUS_POINTS,
    STATUS_FACTOR_X,
    STATUS_FACTOR_Y,
    STATUS_FILE,

    STATUS_PANELS_COUNT,
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setObjectName("mainWindow");
    Ori::Wnd::setWindowIcon(this, ":/window_icons/main");

    _panelDataGrid = new DataGridPanel(this);

    _operations = new Operations(this);
    _operations->getSelectedGraph = [this](){ return selectedGraph(); };
    connect(_operations, &Operations::graphCreated, this, &MainWindow::graphCreated);
    connect(_operations, &Operations::graphUpdated, this, &MainWindow::graphUpdated);

    _mdiArea = new QMdiArea;
    _mdiArea->setBackground(QBrush(QPixmap(":/misc/mdi_background")));
    connect(_mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::mdiSubWindowActivated);
    setCentralWidget(_mdiArea);

    createDocks();
    createActions();
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
}

void MainWindow::loadSettings()
{
    Ori::Settings s;
    s.restoreWindowGeometry(this);
    s.restoreDockState(this);
}

void MainWindow::createActions()
{
#define A_ Ori::Gui::action

    QMenu *m;

    m = menuBar()->addMenu(tr("Plot"));
    _actnPlotNew = m->addAction(tr("New Plot"), this, &MainWindow::newPlot, QKeySequence("Ctrl+N"));
    m->addSeparator();
    m->addAction(tr("Exit"), this, &MainWindow::close);

    m = menuBar()->addMenu(tr("View"));
    connect(m, &QMenu::aboutToShow, this, &MainWindow::updateViewMenu);
    _actnViewTitle = m->addAction(tr("Title"), this, &MainWindow::toggleTitle);
    _actnViewTitle->setCheckable(true);
    _actnViewLegend = m->addAction(tr("Legend"), this, &MainWindow::toggleLegend);
    _actnViewLegend->setCheckable(true);
    m->addSeparator();
    Ori::Gui::makeToggleWidgetsMenu(m, tr("Panels"), {_dockDataGrid});

    m = menuBar()->addMenu(tr("Add"));
    _actnAddFromFile = m->addAction(tr("From File..."), _operations, &Operations::addFromFile, Qt::Key_Insert);
    m->addAction(tr("From File as CSV..."), _operations, &Operations::addFromCsvFile, QKeySequence("Shift+Ctrl+Ins"));
    _actnAddFromClipboard = m->addAction(tr("From Clipboard"), _operations, &Operations::addFromClipboard, QKeySequence("Shift+Ctrl+V"));
    m->addAction(tr("From Clipboard as CSV..."), _operations, &Operations::addFromClipboardCsv, QKeySequence("Ctrl+Alt+V"));
    _actnAddRandomSample = m->addAction(tr("Random Sample"), _operations, &Operations::addRandomSample);

    m = menuBar()->addMenu(tr("Graph"));
    _actnGraphRefresh = m->addAction(tr("Refresh"), _operations, &Operations::graphRefresh, QKeySequence("Ctrl+R"));
    m->addAction(tr("Reopen..."), _operations, &Operations::graphReopen);

    m = menuBar()->addMenu(tr("Modify"));
    _actnModifyOffset = m->addAction(tr("Offset"), _operations, &Operations::modifyOffset, Qt::Key_Plus);
    _actnModifyOffset = m->addAction(tr("Scale"), _operations, &Operations::modifyScale, Qt::Key_Asterisk);

    m = menuBar()->addMenu(tr("Limits"));
    _actnLimitsAuto = m->addAction(tr("Autolimits"), this, &MainWindow::autolimits, QKeySequence("Ctrl+0"));

    m = menuBar()->addMenu(tr("Windows"));
    m->addAction(tr("Cascade"), _mdiArea, &QMdiArea::cascadeSubWindows);
    m->addAction(tr("Tile"), _mdiArea, &QMdiArea::tileSubWindows);

    auto help = Z::HelpSystem::instance();
    auto actnHelpContent = A_(tr("Contents"), help, SLOT(showContents()), ":/toolbar/help", QKeySequence::HelpContents);
    auto actnHelpIndex = A_(tr("Index"), help, SLOT(showIndex()));
    auto actnHelpBugReport = A_(tr("Send Bug Report"), help, SLOT(sendBugReport()), ":/toolbar/bug");
    auto actnHelpUpdates = A_(tr("Check for Updates"), help, SLOT(checkUpdates()), ":/toolbar/update");
    auto actnHelpHomepage = A_(tr("Visit Homepage"), help, SLOT(visitHomePage()), ":/toolbar/home");
    auto actnHelpAbout = A_(tr("About..."), help, SLOT(showAbout()));

    m = menuBar()->addMenu(tr("Help"));
    Ori::Gui::populate(m, {actnHelpContent, actnHelpIndex, nullptr,
                           actnHelpBugReport, actnHelpUpdates, actnHelpHomepage, nullptr,
                           actnHelpAbout});

    auto toolbarMdi = new Ori::Widgets::MdiToolBar(tr("Windows"), _mdiArea);
    toolbarMdi->setMovable(false);
    toolbarMdi->setFloatable(false);
    toolbarMdi->flat = true;
    addToolBar(Qt::BottomToolBarArea, toolbarMdi);

#undef A_
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
    _statusBar = new Ori::Widgets::StatusBar(STATUS_PANELS_COUNT);

    auto versionLabel = new QLabel(Z::HelpSystem::appVersion());
    versionLabel->setContentsMargins(3, 0, 3, 0);
    versionLabel->setForegroundRole(QPalette::Mid);
    _statusBar->addPermanentWidget(versionLabel);

    setStatusBar(_statusBar);
}

PlotWindow* MainWindow::activePlot() const
{
    auto mdiChild = _mdiArea->currentSubWindow();
    if (!mdiChild) return nullptr;
    return dynamic_cast<PlotWindow*>(mdiChild->widget());
}

Graph* MainWindow::selectedGraph() const
{
    auto plot = activePlot();
    return plot ? plot->selectedGraph() : nullptr;
}

void MainWindow::newProject()
{
    newPlot();
}

void MainWindow::newPlot()
{
    auto plotWindow = new PlotWindow();
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

void MainWindow::graphUpdated(Graph* graph) const
{
    for (auto wnd : _mdiArea->subWindowList())
    {
        auto plotWnd = qobject_cast<PlotWindow*>(wnd->widget());
        if (plotWnd && plotWnd->updateGraph(graph))
            return;
    }
}

void MainWindow::graphSelected(Graph* graph) const
{
    if (!graph) return;

    _statusBar->setText(STATUS_POINTS, tr("Points: %1").arg(graph->pointsCount()));

    if (!_panelDataGrid->isVisible()) return;

    auto plot = qobject_cast<PlotWindow*>(sender());
    if (!plot) return;

    _panelDataGrid->showData(plot->plotObj(), graph);
}

void MainWindow::mdiSubWindowActivated(QMdiSubWindow *window) const
{
    if (!_panelDataGrid->isVisible()) return;

    if (!window) return;
    auto plot = qobject_cast<PlotWindow*>(window->widget());
    if (!plot) return;
    auto graph = plot->selectedGraph();
    if (!graph) return;

    // TODO: it resets selection in the data table when the whole application is activated
    _panelDataGrid->showData(plot->plotObj(), graph);
}

void MainWindow::autolimits()
{
    auto plot = activePlot();
    if (plot) plot->autolimits();
}

void MainWindow::updateViewMenu()
{
    auto plot = activePlot();
    _actnViewTitle->setChecked(plot && plot->isTitleVisible());
    _actnViewLegend->setChecked(plot && plot->isLegendVisible());
}

void MainWindow::toggleTitle()
{
    auto plot = activePlot();
    if (plot) plot->setTitleVisible(!plot->isTitleVisible());
}

void MainWindow::toggleLegend()
{
    auto plot = activePlot();
    if (plot) plot->setLegendVisible(!plot->isLegendVisible());
}
