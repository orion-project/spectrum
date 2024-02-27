#include "MainWindow.h"

#include "../app/AppSettings.h"
#include "../app/HelpSystem.h"
#include "../Operations.h"
#include "../core/Graph.h"
#include "../core/DataExporters.h"
#include "../widgets/DataGridPanel.h"
#include "../windows/PlotWindow.h"

#include "helpers/OriWidgets.h"
#include "helpers/OriWindows.h"
#include "tools/OriSettings.h"
#include "widgets/OriLabels.h"
#include "widgets/OriMdiToolBar.h"
#include "widgets/OriPopupMessage.h"
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
#include <QToolBar>

using Ori::Gui::PopupMessage;

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

#define IN_ACTIVE_PLOT(do_func) \
    [this]{ auto plot = activePlot(); if (plot) plot->do_func(); }

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

    //setMenuWidget(toolWidget);

    Ori::MessageBus::instance().registerListener(this);

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

    Ori::Gui::setActionTooltipFormat("<p style='white-space:pre'>%1</p><p><b>%2</b></p>");

    auto menuBar = this->menuBar();

    //---------------------------------------------------------

    auto actPrjNewPlot = A_(tr("New Diagram"), tr("Add new diagram"), this, SLOT(newPlot()), ":/toolbar/plot_new", QKeySequence("Ctrl+N"));
    auto actPrjRenamePlot = A_(tr("Rename Diagram..."), tr("Rename current diagram"), this, SLOT(renamePlot()), ":/toolbar/plot_rename", QKeySequence("Ctrl+F2"));
    auto actPrjDeletePLot = A_(tr("Delete Diagram"), tr("Delete current diagram"), this, SLOT(deletePlot()), ":/toolbar/plot_delete");

    addToolBar(Ori::Gui::toolbar(tr("Project"), "project", {
        actPrjNewPlot, actPrjRenamePlot, actPrjDeletePLot,
    }));

    menuBar->addMenu(Ori::Gui::menu(tr("Project"), this, {
        actPrjNewPlot, actPrjRenamePlot, actPrjDeletePLot,
    }));

    //---------------------------------------------------------

    auto actCopy = A_(tr("Copy"), this, SLOT(editCopy()), ":/toolbar/copy", QKeySequence::Copy);
    auto actPaste = A_(tr("Paste"), this, SLOT(editPaste()), ":/toolbar/paste", QKeySequence::Paste);
    auto actPasteCsv = A_(tr("Paste as CSV..."), _operations, SLOT(addFromClipboardCsv()), ":/toolbar/paste_table");
    auto actCopyFormat = A_(tr("Copy Format"), this, IN_ACTIVE_PLOT(copyPlotFormat), ":/toolbar/copy_fmt", QKeySequence("Ctrl+Shift+C"));
    auto actPasteFormat = A_(tr("Paste Format"), this, IN_ACTIVE_PLOT(pastePlotFormat), ":/toolbar/paste_fmt", QKeySequence("Ctrl+Shift+V"));

    addToolBar(Ori::Gui::toolbar(tr("Edit"), "edit", {
        actCopy, actPaste, actPasteCsv
    }));

    menuBar->addMenu(Ori::Gui::menu(tr("Edit"), this, {
        actCopy, actPaste, actPasteCsv, 0, actCopyFormat, actPasteFormat,
    }));

    //---------------------------------------------------------

    _actToggleDatagrid = A_(tr("Data Grid"), tr("Toggle data grid panel"), this, SLOT(toggleDataGrid()), ":/toolbar/panel_datagrid");
    _actViewTitle = A_(tr("Title"), tr("Toggle diagram title"), this, IN_ACTIVE_PLOT(toggleTitle), ":/toolbar/plot_title");
    _actViewLegend = A_(tr("Legend"), tr("Toggle diagram legend"), this, IN_ACTIVE_PLOT(toggleLegend), ":/toolbar/plot_legend");
    _actToggleDatagrid->setCheckable(true);
    _actViewTitle->setCheckable(true);
    _actViewLegend->setCheckable(true);

    auto menuView = Ori::Gui::menu(tr("View"), this, {
        _actToggleDatagrid, 0, _actViewTitle, _actViewLegend,
    });
    connect(menuView, &QMenu::aboutToShow, this, &MainWindow::viewMenuShown);
    menuBar->addMenu(menuView);

    //---------------------------------------------------------

    auto actAddFile = A_(tr("From File..."), _operations, SLOT(addFromFile()), ":/toolbar/add_file", Qt::Key_Insert);
    auto actAddCsv = A_(tr("From CSV File..."), _operations, SLOT(addFromCsvFile()), ":/toolbar/add_table", QKeySequence("Shift+Ctrl+Ins"));
    auto actAddClipboard = A_(tr("From Clipboard"), _operations, SLOT(addFromClipboard()), ":/toolbar/paste", QKeySequence("Shift+Ctrl+V"));
    auto actAddCsvClipboard = A_(tr("From Clipboard as CSV..."), _operations, SLOT(addFromClipboardCsv()), ":/toolbar/paste_table", QKeySequence("Ctrl+Alt+V"));
    auto actAddRandom = A_(tr("Random Sample"), _operations, SLOT(addRandomSample()), ":/toolbar/add_random");

    addToolBar(Ori::Gui::toolbar(tr("Add"), "add", {
        actAddFile, actAddCsv, 0, actAddRandom,
    }));

    menuBar->addMenu(Ori::Gui::menu(tr("Add"), this, {
        actAddFile, actAddCsv, 0, actAddClipboard, actAddCsvClipboard, 0, actAddRandom,
    }));

    //---------------------------------------------------------

    auto actnGraphRefresh = A_(tr("Refresh"), tr("Reread points from data source"), _operations, SLOT(graphRefresh()), ":/toolbar/update", QKeySequence("Ctrl+R"));
    auto actGraphReopen = A_(tr("Reopen..."), tr("Reselect or reconfigure data source"), _operations, SLOT(graphReopen()), ":/toolbar/update_params");
    auto actGraphTitle = A_(tr("Title..."), tr("Edit title of selected graph"), _operations, SLOT(graphTitle()), ":/toolbar/graph_title", QKeySequence("F2"));
    auto actGraphProps = A_(tr("Properties..."), tr("Set line properties of selected graph"), this, IN_ACTIVE_PLOT(formatGraph), ":/toolbar/graph_props");
    auto actGraphDelete = A_(tr("Delete"), tr("Delete selected graphs"), this, IN_ACTIVE_PLOT(deleteGraph), ":/toolbar/graph_delete");

    // By default the Graph toolbar is in the second row, should be added after all others
    auto tbGraph = Ori::Gui::toolbar(tr("Graph"), "graph", {
        actnGraphRefresh, actGraphReopen, 0, actGraphTitle, actGraphProps, 0, actGraphDelete,
    });

    menuBar->addMenu(Ori::Gui::menu(tr("Graph"), this, {
        actnGraphRefresh, actGraphReopen, 0, actGraphTitle, actGraphProps, 0, actGraphDelete,
    }));

    //---------------------------------------------------------

    auto actOffset = A_(tr("Offset"), _operations, SLOT(modifyOffset()), ":/toolbar/graph_offset", Qt::Key_Plus);
    auto actScale = A_(tr("Scale"), _operations, SLOT(modifyScale()), ":/toolbar/graph_scale", Qt::Key_Asterisk);

    addToolBar(Ori::Gui::toolbar(tr("Modify"), "modify", {
        actOffset, actScale,
    }));

    menuBar->addMenu(Ori::Gui::menu(tr("Modify"), this, {
        actOffset, actScale,
    }));

    //---------------------------------------------------------

    auto actFormatTitle = A_(tr("Title Format..."), this, IN_ACTIVE_PLOT(formatTitle), ":/toolbar/plot_title");
    auto actFormatX = A_(tr("X-axis Format..."), this, IN_ACTIVE_PLOT(formatX), ":/toolbar/format_x");
    auto actFormatY = A_(tr("Y-axis  Format..."), this, IN_ACTIVE_PLOT(formatY), ":/toolbar/format_y");
    auto actFormatLegend = A_(tr("Legend Format..."), this, IN_ACTIVE_PLOT(formatLegend), ":/toolbar/plot_legend");
    auto actFormatGraph = A_(tr("Graph Format..."), this, IN_ACTIVE_PLOT(formatGraph), ":/toolbar/graph_props");
    auto actSavePlotFormat = A_(tr("Save Plot Format..."), this, IN_ACTIVE_PLOT(savePlotFormat), ":/toolbar/save_format");
    auto actLoadPlotFormat = A_(tr("Load Plot Format..."), this, IN_ACTIVE_PLOT(loadPlotFormat), ":/toolbar/open_format");

    auto tbFormat = Ori::Gui::toolbar(tr("Format"), "format", {
        actFormatTitle, actFormatLegend, actFormatX, actFormatY, actFormatGraph,
        0, actSavePlotFormat, actLoadPlotFormat,
    });
    tbFormat->setVisible(false); // hidden by default
    addToolBar(Qt::LeftToolBarArea, tbFormat);

    menuBar->addMenu(Ori::Gui::menu(tr("Format"), this, {
        actFormatTitle, actFormatLegend, actFormatX, actFormatY, actFormatGraph,
        0, actSavePlotFormat, actLoadPlotFormat,
    }));

    //---------------------------------------------------------

    auto actLimitsBoth = A_(tr("Limits for Both Axes..."), this, IN_ACTIVE_PLOT(limitsDlg), ":/toolbar/limits", QKeySequence("Shift+Ctrl+="));
    auto actLimitsX = A_(tr("Limits X..."), this, IN_ACTIVE_PLOT(limitsDlgX), ":/toolbar/limits_x", QKeySequence("Shift+Ctrl+X"));
    auto actLimitsY = A_(tr("Limits Y..."), this, IN_ACTIVE_PLOT(limitsDlgY), ":/toolbar/limits_y", QKeySequence("Shift+Ctrl+Y"));
    auto actAutolimits = A_(tr("Autolimits"), this, IN_ACTIVE_PLOT(autolimits), ":/toolbar/limits_auto", QKeySequence("Alt+0"));
    auto actAutolimitsX = A_(tr("Autolimits X"), this, IN_ACTIVE_PLOT(autolimitsX), ":/toolbar/limits_auto_x", QKeySequence("Alt+X"));
    auto actAutolimitsY = A_(tr("Autolimits Y"), this, IN_ACTIVE_PLOT(autolimitsY), ":/toolbar/limits_auto_y", QKeySequence("Alt+Y"));
    auto actFitSelection = A_(tr("Fit Selection"), this, IN_ACTIVE_PLOT(limitsToSelection), ":/toolbar/limits_fit", QKeySequence("Ctrl+/"));
    auto actFitSelectionX = A_(tr("Fit Selection X"), this, IN_ACTIVE_PLOT(limitsToSelectionX), ":/toolbar/limits_fit_x", QKeySequence("Shift+Ctrl+/,x"));
    auto actFitSelectionY = A_(tr("Fit Selection Y"), this, IN_ACTIVE_PLOT(limitsToSelectionY), ":/toolbar/limits_fit_y", QKeySequence("Shift+Ctrl+/,y"));
    auto actZoomIn = A_(tr("Zoom-in"), this, IN_ACTIVE_PLOT(zoomIn), ":/toolbar/limits_zoom_in", QKeySequence("Ctrl+Alt+="));
    auto actZoomOut = A_(tr("Zoom-out"), this, IN_ACTIVE_PLOT(zoomOut), ":/toolbar/limits_zoom_out", QKeySequence("Ctrl+Alt+-"));
    auto actZoomInX = A_(tr("Zoom-in X"), this, IN_ACTIVE_PLOT(zoomInX), ":/toolbar/limits_zoom_in_x", QKeySequence("Alt+="));
    auto actZoomInY = A_(tr("Zoom-in Y"), this, IN_ACTIVE_PLOT(zoomInY), ":/toolbar/limits_zoom_in_y", QKeySequence("Ctrl+="));
    auto actZoomOutX = A_(tr("Zoom-out X"), this, IN_ACTIVE_PLOT(zoomOutX), ":/toolbar/limits_zoom_out_x", QKeySequence("Alt+-"));
    auto actZoomOutY = A_(tr("Zoom-out Y"), this, IN_ACTIVE_PLOT(zoomOutY), ":/toolbar/limits_zoom_out_y", QKeySequence("Ctrl+-"));

    addToolBar(Qt::RightToolBarArea, Ori::Gui::toolbar(tr("Limits"), "limits", {
        actLimitsBoth, actAutolimits, actFitSelection, actZoomIn, actZoomOut, 0,
        actLimitsX, actAutolimitsX, actFitSelectionX, actZoomInX, actZoomOutX, 0,
        actLimitsY, actAutolimitsY, actFitSelectionY, actZoomInY, actZoomOutY,
    }));

    menuBar->addMenu(Ori::Gui::menu(tr("Limits"), this, {
        actLimitsBoth, actAutolimits, actFitSelection, actZoomIn, actZoomOut, 0,
        actLimitsX, actAutolimitsX, actFitSelectionX, actZoomInX, actZoomOutX, 0,
        actLimitsY, actAutolimitsY, actFitSelectionY, actZoomInY, actZoomOutY,
    }));

    //---------------------------------------------------------

    auto actWndCascade = A_(tr("Cascade"), _mdiArea, SLOT(cascadeSubWindows()), ":/toolbar/wnd_cascade");
    auto actWndTile = A_(tr("Tile"), _mdiArea, SLOT(tileSubWindows()), ":/toolbar/wnd_tile");

    auto menuWindow = Ori::Gui::menu(tr("Windows"), this, {
        actWndCascade, actWndTile,
    });
    menuBar->addMenu(menuWindow);
    // TODO: connect(menuWindow, QMenu::aboutToShow, _mdiArea, QMdiArea::populateWindowMenu);

    //---------------------------------------------------------

    auto help = Z::HelpSystem::instance();
    auto actHelpContent = A_(tr("Help"), help, SLOT(showContent()), ":/toolbar/help", QKeySequence::HelpContents);
    auto actHelpBugReport = A_(tr("Send Bug Report"), help, SLOT(sendBugReport()), ":/toolbar/bug");
    auto actHelpUpdates = A_(tr("Check for Updates"), help, SLOT(checkUpdates()), ":/toolbar/update");
    auto actHelpHomepage = A_(tr("Visit Homepage"), help, SLOT(visitHomePage()), ":/toolbar/home");
    auto actHelpAbout = A_(tr("About..."), help, SLOT(showAbout()), ":/window_icons/main");

    menuBar->addMenu(Ori::Gui::menu(tr("Help"), this, {
        actHelpContent, 0, actHelpBugReport, actHelpUpdates, actHelpHomepage, 0, actHelpAbout,
    }));

    //---------------------------------------------------------

    // By default the Graph toolbar is in the second row
    addToolBarBreak();
    addToolBar(tbGraph);

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

    auto versionLabel = new Ori::Widgets::Label(Z::HelpSystem::appVersion());
    versionLabel->setContentsMargins(3, 0, 3, 0);
    versionLabel->setForegroundRole(QPalette::Mid);
    connect(versionLabel, &Ori::Widgets::Label::doubleClicked, [](){ Z::HelpSystem::instance()->showAbout(); });
    _statusBar->addPermanentWidget(versionLabel);

    setStatusBar(_statusBar);
}

void MainWindow::messageBusEvent(int event, const QMap<QString, QVariant>& params)
{
    switch (event)
    {
    case MSG_PLOT_RENAMED:
        QString plotId = params.value("plotId").toString();
        if (_panelDataGrid->isVisible() && _panelDataGrid->plotId() == plotId)
            _panelDataGrid->showData(findPlotById(plotId), nullptr);
        break;
    }
}

void MainWindow::updateStatusBar()
{

}

PlotWindow* MainWindow::activePlot(bool warn) const
{
    auto mdiChild = _mdiArea->currentSubWindow();
    auto plotWnd = mdiChild ? dynamic_cast<PlotWindow*>(mdiChild->widget()) : nullptr;
    if (!plotWnd and warn)
        PopupMessage::warning(tr("There is no active diagram"));
    return plotWnd;
}

PlotObj* MainWindow::findPlotById(const QString& id) const
{
    foreach (auto wnd, _mdiArea->subWindowList())
    {
        auto plotWnd = qobject_cast<PlotWindow*>(wnd->widget());
        if (plotWnd && plotWnd->plotObj()->id() == id)
            return plotWnd->plotObj();
    }
    return nullptr;
}

Graph* MainWindow::selectedGraph() const
{
    auto plot = activePlot();
    return plot ? plot->selectedGraph() : nullptr;
}

QVector<Graph*> MainWindow::selectedGraphs() const
{
    auto plot = activePlot();
    return plot ? plot->selectedGraphs() : QVector<Graph*>();
}

void MainWindow::newProject()
{
    newPlot();
}

void MainWindow::newPlot()
{
    auto plotWindow = new PlotWindow(_operations);
    connect(plotWindow, &PlotWindow::graphSelected, this, &MainWindow::graphSelected);

    auto mdiChild = _mdiArea->addSubWindow(plotWindow);
    mdiChild->setWindowTitle(plotWindow->windowTitle());
    mdiChild->setWindowIcon(plotWindow->windowIcon());
    mdiChild->show();
}

void MainWindow::renamePlot()
{
    auto plot = activePlot();
    if (plot) plot->rename();
}

void MainWindow::deletePlot()
{
    auto plot = activePlot();
    if (plot)
        _mdiArea->currentSubWindow()->close();
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

    if (AppSettings::autolimitAfterGraphGreated())
        plot->autolimits();
    if (AppSettings::selectNewGraph())
        plot->selectGraph(graph);
}

void MainWindow::graphUpdated(Graph* graph) const
{
    foreach (auto wnd, _mdiArea->subWindowList())
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

    _panelDataGrid->showData(plot->plotObj(), graph);
}

void MainWindow::viewMenuShown()
{
    auto plot = activePlot();
    _actToggleDatagrid->setChecked(_dockDataGrid->isVisible());
    _actViewLegend->setChecked(plot && plot->isLegendVisible());
    _actViewTitle->setChecked(plot && plot->isTitleVisible());
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
        PopupMessage::affirm(tr("Data grid values has been copied to Clipboard"));
        return;
    }

    auto plot = activePlot();
    if (!plot) return;

    auto graph = plot->selectedGraph();
    if (!graph)
    {
        // There will be popup message after copying image
        plot->copyPlotImage();
        return;
    }

    // TODO: show popup message if there is no dialog here, otherwise leave a comment
    DataExporters::copyToClipboard(graph->data());
}

void MainWindow::editPaste()
{
    auto plot = activePlot();
    if (plot) _operations->addFromClipboard();
}
