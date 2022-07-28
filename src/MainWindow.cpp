#include "MainWindow.h"

#include "HelpSystem.h"
#include "Operations.h"
#include "PlotWindow.h"
#include "Settings.h"
#include "core/Graph.h"
#include "core/DataExporters.h"
#include "widgets/DataGridPanel.h"

#include "helpers/OriWindows.h"
#include "helpers/OriWidgets.h"
#include "tools/OriSettings.h"
#include "widgets/OriFlatToolBar.h"
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
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>

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

    _toolTabs = new QTabWidget;

    auto mainToolbar = new Ori::Widgets::FlatToolBar;
    mainToolbar->addWidget(_toolTabs);
    mainToolbar->setFloatable(false);
    mainToolbar->setAllowedAreas(Qt::TopToolBarArea);
    mainToolbar->setMovable(false);
    addToolBar(mainToolbar);

    setCentralWidget(_mdiArea);

    createDocks();
    createActions();
    createStatusBar();

    loadSettings();

    QTimer::singleShot(200, this, [this](){ this->newProject(); });
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
#define T_ Ori::Gui::textToolButton

    QMenu *m;

    //---------------------------------------------------------

    auto actNewPlot = A_(tr("New Plot"), this, SLOT(newPlot()), ":/toolbar/plot_new", QKeySequence("Ctrl+N"));

    auto menuProject = menuBar()->addMenu(tr("Project"));
    Ori::Gui::populate(menuProject, {
                           actNewPlot,
                       });
    menuProject->addSeparator();
    menuProject->addAction(tr("Exit"), this, &MainWindow::close);

    auto toolbarProject = new Ori::Widgets::FlatToolBar;
    Ori::Gui::populate(toolbarProject, {
                           T_(actNewPlot),
                       });
    _toolTabs->addTab(toolbarProject, menuProject->title());

    //---------------------------------------------------------

    m = menuBar()->addMenu(tr("View"));
    connect(m, &QMenu::aboutToShow, this, &MainWindow::updateViewMenu);
    _actnViewTitle = m->addAction(tr("Title"), this, &MainWindow::toggleTitle);
    _actnViewTitle->setCheckable(true);
    _actnViewLegend = m->addAction(tr("Legend"), this, &MainWindow::toggleLegend);
    _actnViewLegend->setCheckable(true);
    m->addSeparator();
    Ori::Gui::makeToggleWidgetsMenu(m, tr("Panels"), {_dockDataGrid});

    //---------------------------------------------------------

    auto actCopy = A_(tr("Copy"), this, SLOT(editCopy()), ":/toolbar/copy", QKeySequence::Copy);
    auto actPaste = A_(tr("Paste"), this, SLOT(editPaste()), ":/toolbar/paste", QKeySequence::Paste);

    auto menuEdit = menuBar()->addMenu(tr("Edit"));
    Ori::Gui::populate(menuEdit, {
                           actCopy, actPaste,
                       });

    auto toolbarEdit = new Ori::Widgets::FlatToolBar;
    Ori::Gui::populate(toolbarEdit, {
                           actCopy, actPaste,
                       });
    _toolTabs->addTab(toolbarEdit, menuEdit->title());

    //---------------------------------------------------------

    auto actAddFile = A_(tr("From File..."), _operations, SLOT(addFromFile()), ":/toolbar/add_file", Qt::Key_Insert);
    auto actAddClipboard = A_(tr("From Clipboard"), _operations, SLOT(addFromClipboard()), ":/toolbar/paste", QKeySequence("Shift+Ctrl+V"));
    auto actAddCsv = A_(tr("From File as CSV..."), _operations, SLOT(addFromCsvFile()), ":/toolbar/add_table", QKeySequence("Shift+Ctrl+Ins"));
    auto actAddCsvClipboard = A_(tr("From Clipboard as CSV..."), _operations, SLOT(addFromClipboardCsv()), ":/toolbar/paste_table", QKeySequence("Ctrl+Alt+V"));
    auto actAddRandom = A_(tr("Random Sample"), _operations, SLOT(addRandomSample()), ":/toolbar/add_random");

    auto menuAdd = menuBar()->addMenu(tr("Add"));
    Ori::Gui::populate(menuAdd, {
                           actAddFile, actAddClipboard, actAddCsv, actAddCsvClipboard, actAddRandom,
                       });

    auto toolbarAdd = new Ori::Widgets::FlatToolBar;
    Ori::Gui::populate(toolbarAdd, {
                           T_(actAddFile), T_(actAddClipboard), T_(actAddCsv), T_(actAddCsvClipboard), T_(actAddRandom),
                       });
    _toolTabs->addTab(toolbarAdd, menuAdd->title());

    //---------------------------------------------------------

    auto actnGraphRefresh = A_(tr("Refresh"), _operations, SLOT(graphRefresh()), ":/toolbar/todo", QKeySequence("Ctrl+R"));
    auto actGraphReopen = A_(tr("Reopen..."), _operations, SLOT(graphReopen()), ":/toolbar/todo");

    auto menuGraph = menuBar()->addMenu(tr("Graph"));
    Ori::Gui::populate(menuGraph, {
                           actnGraphRefresh, actGraphReopen,
                       });

    auto toolbarGraph = new Ori::Widgets::FlatToolBar;
    Ori::Gui::populate(toolbarGraph, {
                           T_(actnGraphRefresh), nullptr,
                           T_(actGraphReopen), nullptr
                       });
    _toolTabs->addTab(toolbarGraph, menuGraph->title());


    //---------------------------------------------------------

    auto actOffset = A_(tr("Offset"), _operations, SLOT(modifyOffset()), ":/toolbar/graph_offset", Qt::Key_Plus);
    auto actScale = A_(tr("Scale"), _operations, SLOT(modifyScale()), ":/toolbar/graph_scale", Qt::Key_Asterisk);

    auto menuModify = menuBar()->addMenu(tr("Modify"));
    Ori::Gui::populate(menuModify, {
                           actOffset, nullptr,
                           actScale, nullptr
                       });

    auto toolbarModify = new Ori::Widgets::FlatToolBar;
    Ori::Gui::populate(toolbarModify, {
                           T_(actOffset), nullptr,
                           T_(actScale), nullptr
                       });
    _toolTabs->addTab(toolbarModify, menuModify->title());

    //---------------------------------------------------------

    auto actLimitsBoth = A_(tr("Limits for Both Axes..."), this, SLOT(limitsDlg()), ":/toolbar/todo", QKeySequence("Shift+Ctrl+="));
    auto actLimitsX = A_(tr("Limits for X-axis..."), this, SLOT(limitsDlgX()), ":/toolbar/todo", QKeySequence("Shift+Ctrl+X"));
    auto actLimitsY = A_(tr("Limits for Y-axis..."), this, SLOT(limitsDlgY()), ":/toolbar/todo", QKeySequence("Shift+Ctrl+Y"));
    auto actAutolimits = A_(tr("Autolimits"), this, SLOT(autolimits()), ":/toolbar/limits_auto", QKeySequence("Alt+0"));
    auto actAutolimitsX = A_(tr("Autolimits over X"), this, SLOT(autolimitsX()), ":/toolbar/limits_auto_x", QKeySequence("Alt+X"));
    auto actAutolimitsY = A_(tr("Autolimits over Y"), this, SLOT(autolimitsY()), ":/toolbar/limits_auto_y", QKeySequence("Alt+Y"));
    auto actFitSelection = A_(tr("Fit Selection"), this, SLOT(limitsToSelection()), ":/toolbar/todo", QKeySequence("Ctrl+/"));
    auto actFitSelectionX = A_(tr("Fit Selection over X"), this, SLOT(limitsToSelectionX()), ":/toolbar/todo", QKeySequence("Shift+Ctrl+/,x"));
    auto actFitSelectionY = A_(tr("Fit Selection over Y"), this, SLOT(limitsToSelectionY()), ":/toolbar/todo", QKeySequence("Shift+Ctrl+/,y"));
    auto actZoomIn = A_(tr("Zoom-in"), this, SLOT(zoomIn()), ":/toolbar/limits_zoom_in", QKeySequence("Ctrl+Alt+="));
    auto actZoomOut = A_(tr("Zoom-out"), this, SLOT(zoomOut()), ":/toolbar/limits_zoom_out", QKeySequence("Ctrl+Alt+-"));
    auto actZoomInX = A_(tr("Zoom-in over X"), this, SLOT(zoomInX()), ":/toolbar/limits_zoom_in_x", QKeySequence("Alt+="));
    auto actZoomInY = A_(tr("Zoom-in over Y"), this, SLOT(zoomInY()), ":/toolbar/limits_zoom_in_y", QKeySequence("Ctrl+="));
    auto actZoomOutX = A_(tr("Zoom-out over X"), this, SLOT(zoomOutX()), ":/toolbar/limits_zoom_out_x", QKeySequence("Alt+-"));
    auto actZoomOutY = A_(tr("Zoom-out over Y"), this, SLOT(zoomOutY()), ":/toolbar/limits_zoom_out_y", QKeySequence("Ctrl+-"));

    auto menuLimits = menuBar()->addMenu(tr("Limits"));
    Ori::Gui::populate(menuLimits, {
                           actLimitsBoth, actLimitsX, actLimitsY, nullptr,
                           actAutolimits, actAutolimitsX, actAutolimitsY, nullptr,
                           actFitSelection, actFitSelectionX, actFitSelectionY, nullptr,
                           actZoomIn, actZoomOut, actZoomInX, actZoomOutX, actZoomInY, actZoomOutY,
                       });

    auto toolbarLimits = new Ori::Widgets::FlatToolBar;
    Ori::Gui::populate(toolbarLimits, {
                           actLimitsBoth, actLimitsX, actLimitsY, nullptr,
                           actAutolimits, actAutolimitsX, actAutolimitsY, nullptr,
                           actFitSelection, actFitSelectionX, actFitSelectionY, nullptr,
                           actZoomIn, actZoomOut, nullptr, actZoomInX, actZoomOutX, nullptr, actZoomInY, actZoomOutY,
                       });
    _toolTabs->addTab(toolbarLimits, menuLimits->title());

    //---------------------------------------------------------

    m = menuBar()->addMenu(tr("Windows"));
    m->addAction(tr("Cascade"), _mdiArea, &QMdiArea::cascadeSubWindows);
    m->addAction(tr("Tile"), _mdiArea, &QMdiArea::tileSubWindows);

    //---------------------------------------------------------

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

    //---------------------------------------------------------

    auto toolbarMdi = new Ori::Widgets::MdiToolBar(tr("Windows"), _mdiArea);
    toolbarMdi->setMovable(false);
    toolbarMdi->setFloatable(false);
    toolbarMdi->flat = true;
    addToolBar(Qt::BottomToolBarArea, toolbarMdi);

#undef A_
#undef T_
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

void MainWindow::limitsDlg()
{
    auto plot = activePlot();
    if (plot) plot->limitsDlg();
}

void MainWindow::limitsDlgX()
{
    auto plot = activePlot();
    if (plot) plot->limitsDlgX();
}

void MainWindow::limitsDlgY()
{
    auto plot = activePlot();
    if (plot) plot->limitsDlgY();
}

void MainWindow::autolimits()
{
    auto plot = activePlot();
    if (plot) plot->autolimits();
}

void MainWindow::autolimitsX()
{
    auto plot = activePlot();
    if (plot) plot->autolimitsX();
}

void MainWindow::autolimitsY()
{
    auto plot = activePlot();
    if (plot) plot->autolimitsY();
}

void MainWindow::limitsToSelection()
{
    auto plot = activePlot();
    if (plot) plot->limitsToSelection();
}

void MainWindow::limitsToSelectionX()
{
    auto plot = activePlot();
    if (plot) plot->limitsToSelectionX();
}

void MainWindow::limitsToSelectionY()
{
    auto plot = activePlot();
    if (plot) plot->limitsToSelectionY();
}

void MainWindow::zoomIn()
{
    auto plot = activePlot();
    if (plot) plot->zoomIn();
}

void MainWindow::zoomOut()
{
    auto plot = activePlot();
    if (plot) plot->zoomOut();
}

void MainWindow::zoomInX()
{
    auto plot = activePlot();
    if (plot) plot->zoomInX();
}

void MainWindow::zoomOutX()
{
    auto plot = activePlot();
    if (plot) plot->zoomOutX();
}

void MainWindow::zoomInY()
{
    auto plot = activePlot();
    if (plot) plot->zoomInY();
}

void MainWindow::zoomOutY()
{
    auto plot = activePlot();
    if (plot) plot->zoomOutY();
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

void MainWindow::editCopy()
{
    if (_panelDataGrid->isVisible() && _panelDataGrid->hasFocus())
    {
        _panelDataGrid->copyData();
        return;
    }

    auto plot = activePlot();
    if (plot)
    {
        auto graph = plot->selectedGraph();
        if (graph)
        {
            DataExporters::copyToClipboard(graph->data());
            return;
        }
    }
}

void MainWindow::editPaste()
{
    auto plot = activePlot();
    if (plot)
    {
        _operations->addFromClipboard();
        return;
    }
}
