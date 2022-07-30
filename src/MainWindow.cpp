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
#include "helpers/OriLayouts.h"
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
    auto toolWidget = Ori::Layouts::LayoutV({_toolTabs}).setMargin(3).makeWidget();

    setCentralWidget(_mdiArea);

    createDocks();
    createActions();
    createStatusBar();

    loadSettings();

    setMenuWidget(toolWidget);

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

void MainWindow::createTools(QString title, std::initializer_list<QObject*> items)
{
    auto t = new Ori::Widgets::FlatToolBar;
    Ori::Gui::populate(t, items);
    _toolTabs->addTab(t, title);
}

void MainWindow::createActions()
{
#define A_ Ori::Gui::action
#define T_ Ori::Gui::textToolButton

    Ori::Gui::setActionTooltipFormat("<p style='white-space:pre'>%1</p><p><b>%2</b></p>");

    //---------------------------------------------------------

    auto actNewPlot = A_(tr("New"), tr("Add new plot"), this, SLOT(newPlot()), ":/toolbar/plot_new", QKeySequence("Ctrl+N"));
    auto actRenamePlot = A_(tr("Rename..."), tr("Rename current plot"), this, SLOT(renamePlot()), ":/toolbar/plot_rename", QKeySequence("Ctrl+F2"));

    createTools(tr("Plot"), {
                    T_(actNewPlot), T_(actRenamePlot),
                });

    //---------------------------------------------------------

    auto actToggleDatagrid = A_(tr("Data Grid"), tr("Toggle data grid panel"), this, SLOT(toggleDataGrid()), ":/toolbar/panel_datagrid");
    auto actViewTitle = A_(tr("Title"), tr("Toggle plot title visibility"), this, SLOT(toggleTitle()), ":/toolbar/plot_title");
    auto actViewLegend = A_(tr("Legend"), tr("Toggle plot legend visibility"), this, SLOT(toggleLegend()), ":/toolbar/plot_legend");

    createTools(tr("View"), {
                    T_(actToggleDatagrid), nullptr,
                    T_(actViewTitle), T_(actViewLegend),
                });

    //---------------------------------------------------------

    auto actCopy = A_(tr("Copy"), this, SLOT(editCopy()), ":/toolbar/copy", QKeySequence::Copy);
    auto actPaste = A_(tr("Paste"), this, SLOT(editPaste()), ":/toolbar/paste", QKeySequence::Paste);
    auto actPasteCsv = A_(tr("Paste as CSV..."), _operations, SLOT(addFromClipboardCsv()), ":/toolbar/paste_table");

    createTools(tr("Edit"), {
                    actCopy, actPaste, T_(actPasteCsv),
                });

    //---------------------------------------------------------

    auto actAddFile = A_(tr("From File..."), _operations, SLOT(addFromFile()), ":/toolbar/add_file", Qt::Key_Insert);
    auto actAddClipboard = A_(tr("From Clipboard"), _operations, SLOT(addFromClipboard()), ":/toolbar/paste", QKeySequence("Shift+Ctrl+V"));
    auto actAddCsv = A_(tr("From CSV File..."), _operations, SLOT(addFromCsvFile()), ":/toolbar/add_table", QKeySequence("Shift+Ctrl+Ins"));
    auto actAddCsvClipboard = A_(tr("From Clipboard as CSV..."), _operations, SLOT(addFromClipboardCsv()), ":/toolbar/paste_table", QKeySequence("Ctrl+Alt+V"));
    auto actAddRandom = A_(tr("Random Sample"), _operations, SLOT(addRandomSample()), ":/toolbar/add_random");

    createTools(tr("Add"), {
                    T_(actAddFile), T_(actAddClipboard), T_(actAddCsv), T_(actAddCsvClipboard), T_(actAddRandom),
                });

    //---------------------------------------------------------

    auto actnGraphRefresh = A_(tr("Refresh"), tr("Reread points from data source"), _operations, SLOT(graphRefresh()), ":/toolbar/update", QKeySequence("Ctrl+R"));
    auto actGraphReopen = A_(tr("Reopen..."), tr("Reselect or reconfigure data source"), _operations, SLOT(graphReopen()), ":/toolbar/update_params");

    createTools(tr("Graph"), {
                    T_(actnGraphRefresh), nullptr,
                    T_(actGraphReopen), nullptr
                });

    //---------------------------------------------------------

    auto actOffset = A_(tr("Offset"), _operations, SLOT(modifyOffset()), ":/toolbar/graph_offset", Qt::Key_Plus);
    auto actScale = A_(tr("Scale"), _operations, SLOT(modifyScale()), ":/toolbar/graph_scale", Qt::Key_Asterisk);

    createTools(tr("Modify"), {
                    T_(actOffset), nullptr,
                    T_(actScale), nullptr
                });

    //---------------------------------------------------------

    auto actLimitsBoth = A_(tr("Limits for Both Axes..."), this, SLOT(limitsDlg()), ":/toolbar/limits", QKeySequence("Shift+Ctrl+="));
    auto actLimitsX = A_(tr("Limits X..."), this, SLOT(limitsDlgX()), ":/toolbar/limits_x", QKeySequence("Shift+Ctrl+X"));
    auto actLimitsY = A_(tr("Limits Y..."), this, SLOT(limitsDlgY()), ":/toolbar/limits_y", QKeySequence("Shift+Ctrl+Y"));
    auto actAutolimits = A_(tr("Autolimits"), this, SLOT(autolimits()), ":/toolbar/limits_auto", QKeySequence("Alt+0"));
    auto actAutolimitsX = A_(tr("Autolimits X"), this, SLOT(autolimitsX()), ":/toolbar/limits_auto_x", QKeySequence("Alt+X"));
    auto actAutolimitsY = A_(tr("Autolimits Y"), this, SLOT(autolimitsY()), ":/toolbar/limits_auto_y", QKeySequence("Alt+Y"));
    auto actFitSelection = A_(tr("Fit Selection"), this, SLOT(limitsToSelection()), ":/toolbar/limits_fit", QKeySequence("Ctrl+/"));
    auto actFitSelectionX = A_(tr("Fit Selection X"), this, SLOT(limitsToSelectionX()), ":/toolbar/limits_fit_x", QKeySequence("Shift+Ctrl+/,x"));
    auto actFitSelectionY = A_(tr("Fit Selection Y"), this, SLOT(limitsToSelectionY()), ":/toolbar/limits_fit_y", QKeySequence("Shift+Ctrl+/,y"));
    auto actZoomIn = A_(tr("Zoom-in"), this, SLOT(zoomIn()), ":/toolbar/limits_zoom_in", QKeySequence("Ctrl+Alt+="));
    auto actZoomOut = A_(tr("Zoom-out"), this, SLOT(zoomOut()), ":/toolbar/limits_zoom_out", QKeySequence("Ctrl+Alt+-"));
    auto actZoomInX = A_(tr("Zoom-in X"), this, SLOT(zoomInX()), ":/toolbar/limits_zoom_in_x", QKeySequence("Alt+="));
    auto actZoomInY = A_(tr("Zoom-in Y"), this, SLOT(zoomInY()), ":/toolbar/limits_zoom_in_y", QKeySequence("Ctrl+="));
    auto actZoomOutX = A_(tr("Zoom-out X"), this, SLOT(zoomOutX()), ":/toolbar/limits_zoom_out_x", QKeySequence("Alt+-"));
    auto actZoomOutY = A_(tr("Zoom-out Y"), this, SLOT(zoomOutY()), ":/toolbar/limits_zoom_out_y", QKeySequence("Ctrl+-"));

    createTools(tr("Limits"), {
                    actLimitsBoth, T_(actAutolimits), T_(actFitSelection), actZoomIn, actZoomOut, nullptr,
                    T_(actLimitsX), actAutolimitsX, actFitSelectionX, actZoomInX, actZoomOutX, nullptr,
                    T_(actLimitsY), actAutolimitsY, actFitSelectionY, actZoomInY, actZoomOutY,
                });

    //---------------------------------------------------------

    auto actWndCascade = A_(tr("Cascade"), _mdiArea, SLOT(cascadeSubWindows()), ":/toolbar/wnd_cascade");
    auto actWndTile = A_(tr("Tile"), _mdiArea, SLOT(tileSubWindows()), ":/toolbar/wnd_tile");

    createTools(tr("Windows"), {
                    T_(actWndCascade), T_(actWndTile),
                });

    //---------------------------------------------------------

    auto help = Z::HelpSystem::instance();
    auto actHelpIndex = A_(tr("Index"), help, SLOT(showIndex()), ":/toolbar/help", QKeySequence::HelpContents);
    auto actHelpBugReport = A_(tr("Send Bug Report"), help, SLOT(sendBugReport()), ":/toolbar/bug");
    auto actHelpUpdates = A_(tr("Check for Updates"), help, SLOT(checkUpdates()), ":/toolbar/update");
    auto actHelpHomepage = A_(tr("Visit Homepage"), help, SLOT(visitHomePage()), ":/toolbar/home");
    auto actHelpAbout = A_(tr("About..."), help, SLOT(showAbout()), ":/window_icons/main");

    createTools(tr("Help"), {
                    T_(actHelpIndex), nullptr,
                    T_(actHelpBugReport), T_(actHelpUpdates), T_(actHelpHomepage), nullptr,
                    T_(actHelpAbout),
                });

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

void MainWindow::renamePlot()
{
    // TODO
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

void MainWindow::toggleDataGrid()
{
    _dockDataGrid->setVisible(!_dockDataGrid->isVisible());
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
