#include "MainWindow.h"

#include "../app/AppSettings.h"
#include "../app/HelpSystem.h"
#include "../Operations.h"
#include "../core/Graph.h"
#include "../core/DataExporters.h"
#include "../core/DataSources.h"
#include "../widgets/DataGridPanel.h"
#include "../windows/PlotWindow.h"

#include "helpers/OriWidgets.h"
#include "helpers/OriWindows.h"
#include "tools/OriMruList.h"
#include "tools/OriSettings.h"
#include "widgets/OriLabels.h"
#include "widgets/OriMdiToolBar.h"
#include "widgets/OriMruMenu.h"
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
    STATUS_PLOTS,
    STATUS_GRAPHS,
    STATUS_MODIF,
    STATUS_POINTS,
    STATUS_FACTOR_X,
    STATUS_FACTOR_Y,
    STATUS_DATA_SOURCE,

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
    connect(_operations->mruPlotFormats(), &Ori::MruFileList::clicked, this, [this](const QString& fileName){
        auto plot = activePlot(); if (plot) plot->loadPlotFormat(fileName);
    });

    _mdiArea = new QMdiArea;
    _mdiArea->setBackground(QBrush(QPixmap(":/misc/mdi_background")));
    connect(_mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::mdiSubWindowActivated);

    setCentralWidget(_mdiArea);

    createDocks();
    createActions();
    createStatusBar();

    restoreState();

    Ori::MessageBus::instance().registerListener(this);

    QTimer::singleShot(200, this, [this](){ newPlot(); });
}

MainWindow::~MainWindow()
{
    storeState();
}

void MainWindow::storeState()
{
    Ori::Settings s;
    s.storeWindowGeometry(this);
    s.storeDockState(this);
}

void MainWindow::restoreState()
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

    auto actPrjNewPlot = A_(tr("New Diagram"), this, SLOT(newPlot()), ":/toolbar/plot_new", QKeySequence("Ctrl+N"));
    auto actPrjRenamePlot = A_(tr("Rename Diagram..."), this, IN_ACTIVE_PLOT(rename), ":/toolbar/plot_rename", QKeySequence("Ctrl+F2"));
    auto actPrjDeletePlot = A_(tr("Delete Diagram"), this, SLOT(deletePlot()), ":/toolbar/plot_delete");
    auto actSavePlotImg = A_(tr("Save Diagram as Image..."), this, IN_ACTIVE_PLOT(exportPlotImg), ":/toolbar/save_img");
    auto actExit = A_(tr("Exit"), this, SLOT(close()));

    menuBar->addMenu(Ori::Gui::menu(tr("Project"), this, {
        actPrjNewPlot, actPrjRenamePlot, actPrjDeletePlot, actSavePlotImg,
        0, actExit
    }));

    addToolBar(Ori::Gui::toolbar(tr("Project"), "project", {
        actPrjNewPlot, actPrjRenamePlot, actPrjDeletePlot, actSavePlotImg
    }));

    //---------------------------------------------------------

    auto actCopy = A_(tr("Copy"), tr("Copy graph data or image"), this, SLOT(editCopy()), ":/toolbar/copy", QKeySequence::Copy);
    auto actPaste = A_(tr("Paste"), tr("Create graph from Clipboard data"), _operations, SLOT(addFromClipboard()), ":/toolbar/paste", QKeySequence::Paste);
    auto actPasteCsv = A_(tr("Paste as CSV..."), tr("Create graphs from Clipboard data (with config dialog)"), _operations, SLOT(addFromClipboardCsv()), ":/toolbar/paste_table", QKeySequence("Ctrl+Alt+V"));
    auto actCopyFormat = A_(tr("Copy Format"), tr("Copy diagram format"), this, IN_ACTIVE_PLOT(copyPlotFormat), ":/toolbar/copy_fmt", QKeySequence("Ctrl+Shift+C"));
    auto actPasteFormat = A_(tr("Paste Format"), tr("Paste diagram format"), this, IN_ACTIVE_PLOT(pastePlotFormat), ":/toolbar/paste_fmt", QKeySequence("Ctrl+Shift+V"));

    menuBar->addMenu(Ori::Gui::menu(tr("Edit"), this, {
        actCopy, actPaste, actPasteCsv, 0, actCopyFormat, actPasteFormat,
    }));

    addToolBar(Ori::Gui::toolbar(tr("Edit"), "edit", {
        actCopy, actPaste, actPasteCsv
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

    auto actAddFile = A_(tr("From File..."), tr("Create graph from file data"), _operations, SLOT(addFromFile()), ":/toolbar/add_file", Qt::Key_Insert);
    auto actAddCsv = A_(tr("From CSV File..."), tr("Create graphs from file (with config dialog)"), _operations, SLOT(addFromCsvFile()), ":/toolbar/add_table", QKeySequence("Shift+Ctrl+Ins"));
    auto actAddClipboard = A_(tr("From Clipboard"), tr("Create graph from Clipboard data"), _operations, SLOT(addFromClipboard()), ":/toolbar/paste");
    auto actAddCsvClipboard = A_(tr("From Clipboard as CSV..."), tr("Create graphs from Clipboard data (with config dialog)"), _operations, SLOT(addFromClipboardCsv()), ":/toolbar/paste_table");
    auto actAddRandom = A_(tr("Random Sample..."), tr("Add random sample graph"), _operations, SLOT(addRandomSample()), ":/toolbar/add_random");

    menuBar->addMenu(Ori::Gui::menu(tr("Add"), this, {
        actAddFile, actAddCsv, 0, actAddClipboard, actAddCsvClipboard, 0, actAddRandom,
    }));

    addToolBar(Ori::Gui::toolbar(tr("Add"), "add", {
        actAddFile, actAddCsv, 0, actAddRandom,
    }));

    //---------------------------------------------------------

    auto actnGraphRefresh = A_(tr("Refresh"), tr("Reread points from data source"), _operations, SLOT(graphRefresh()), ":/toolbar/update", QKeySequence("Ctrl+R"));
    auto actGraphReopen = A_(tr("Reopen..."), tr("Reselect or reconfigure data source"), _operations, SLOT(graphReopen()), ":/toolbar/update_params");
    auto actGraphTitle = A_(tr("Title..."), tr("Edit title of selected graph"), this, IN_ACTIVE_PLOT(renameGraph), ":/toolbar/graph_title", QKeySequence("F2"));
    auto actGraphProps = A_(tr("Line Format..."), tr("Set line format of selected graph"), this, IN_ACTIVE_PLOT(formatGraph), ":/toolbar/graph_format");
    auto actGraphDelete = A_(tr("Delete"), tr("Delete selected graphs"), this, IN_ACTIVE_PLOT(deleteGraph), ":/toolbar/graph_delete");
    auto actGraphAxes = A_(tr("Change Axes..."), this, IN_ACTIVE_PLOT(changeGraphAxes));

    menuBar->addMenu(Ori::Gui::menu(tr("Graph"), this, {
        actnGraphRefresh, actGraphReopen, 0, actGraphTitle, actGraphProps, actGraphAxes, 0, actGraphDelete,
    }));

    // By default the Graph toolbar is in the second row, should be added after all others
    auto tbGraph = Ori::Gui::toolbar(tr("Graph"), "graph", {
        actnGraphRefresh, actGraphReopen, 0, actGraphTitle, actGraphProps, 0, actGraphDelete,
    });

    //---------------------------------------------------------

    auto actOffset = A_(tr("Offset"), _operations, SLOT(modifyOffset()), ":/toolbar/graph_offset", Qt::Key_Plus);
    auto actScale = A_(tr("Scale"), _operations, SLOT(modifyScale()), ":/toolbar/graph_scale", Qt::Key_Asterisk);

    menuBar->addMenu(Ori::Gui::menu(tr("Modify"), this, {
        actOffset, actScale,
    }));

    addToolBar(Ori::Gui::toolbar(tr("Modify"), "modify", {
        actOffset, actScale,
    }));

    //---------------------------------------------------------

    auto actFormatTitle = A_(tr("Title Format..."), this, IN_ACTIVE_PLOT(formatTitle), ":/toolbar/plot_title");
    auto actFormatX = A_(tr("Bottom Axis Format..."), this, IN_ACTIVE_PLOT(formatX), ":/toolbar/format_x");
    auto actFormatY = A_(tr("Left Axis Format..."), this, IN_ACTIVE_PLOT(formatY), ":/toolbar/format_y");
    auto actFormatX2 = A_(tr("Top Axis Format..."), this, IN_ACTIVE_PLOT(formatX2));
    auto actFormatY2 = A_(tr("Right Axis Format..."), this, IN_ACTIVE_PLOT(formatY2));
    auto actFormatAxis = A_(tr("Format Axis..."), this, IN_ACTIVE_PLOT(formatAxis), ":/toolbar/format_axis");
    auto actFactorAxis = A_(tr("Set Axis Factor..."), this, IN_ACTIVE_PLOT(axisFactorDlg), ":/toolbar/factor_axis");
    auto actFormatLegend = A_(tr("Legend Format..."), this, IN_ACTIVE_PLOT(formatLegend), ":/toolbar/plot_legend");
    auto actSavePlotFormat = A_(tr("Save Plot Format..."), this, IN_ACTIVE_PLOT(savePlotFormatDlg), ":/toolbar/save_format");
    auto actLoadPlotFormat = A_(tr("Load Plot Format..."), this, IN_ACTIVE_PLOT(loadPlotFormatDlg), ":/toolbar/open_format");

    auto menuAddAxis = Ori::Gui::menu(tr("Add Axis"), {
        A_(tr("At Bottom"), this, IN_ACTIVE_PLOT(addAxisBottom), ":/toolbar/axis_bottom"),
        A_(tr("At Left"), this, IN_ACTIVE_PLOT(addAxisLeft), ":/toolbar/axis_left"),
        A_(tr("At Top"), this, IN_ACTIVE_PLOT(addAxisTop), ":/toolbar/axis_top"),
        A_(tr("At Right"), this, IN_ACTIVE_PLOT(addAxisRight), ":/toolbar/axis_right"),
    });

    menuBar->addMenu(Ori::Gui::menu(tr("Format"), this, {
        actFormatTitle, actFormatLegend,
        0, actFormatX, actFormatY, actFormatX2, actFormatY2,
        0, actFormatAxis, actFactorAxis, menuAddAxis,
        0, actSavePlotFormat, actLoadPlotFormat,
        new Ori::Widgets::MruMenu(tr("Recent Formats"), _operations->mruPlotFormats()),
    }));

    auto tbFormat = Ori::Gui::toolbar(tr("Format"), "format", {
        actFormatTitle, actFormatLegend,
        0, actFormatAxis, actFactorAxis,
        //0, actFormatX, actFormatY,
        //0, actFactorX, actFactorY,
        0, actSavePlotFormat, actLoadPlotFormat,
    });
    tbFormat->setVisible(false); // hidden by default
    addToolBar(Qt::LeftToolBarArea, tbFormat);

    //---------------------------------------------------------

    auto actLimitsBoth = A_(tr("Limits for Both Axes..."), this, IN_ACTIVE_PLOT(limitsDlg), ":/toolbar/limits", QKeySequence("Shift+Ctrl+="));
    auto actLimitsX = A_(tr("Limits X..."), this, IN_ACTIVE_PLOT(limitsDlgX), ":/toolbar/limits_x", QKeySequence("Shift+Ctrl+X"));
    auto actLimitsY = A_(tr("Limits Y..."), this, IN_ACTIVE_PLOT(limitsDlgY), ":/toolbar/limits_y", QKeySequence("Shift+Ctrl+Y"));
    auto actAutolimits = A_(tr("Autolimits"), this, IN_ACTIVE_PLOT(autolimits), ":/toolbar/limits_auto", QKeySequence("Alt+0"));
    auto actAutolimitsX = A_(tr("Autolimits X"), this, IN_ACTIVE_PLOT(autolimitsX), ":/toolbar/limits_auto_x", QKeySequence("Alt+X"));
    auto actAutolimitsY = A_(tr("Autolimits Y"), this, IN_ACTIVE_PLOT(autolimitsY), ":/toolbar/limits_auto_y", QKeySequence("Alt+Y"));
//    auto actFitSelection = A_(tr("Fit Selection"), this, IN_ACTIVE_PLOT(limitsToSelection), ":/toolbar/limits_fit", QKeySequence("Ctrl+/"));
//    auto actFitSelectionX = A_(tr("Fit Selection X"), this, IN_ACTIVE_PLOT(limitsToSelectionX), ":/toolbar/limits_fit_x", QKeySequence("Shift+Ctrl+/,x"));
//    auto actFitSelectionY = A_(tr("Fit Selection Y"), this, IN_ACTIVE_PLOT(limitsToSelectionY), ":/toolbar/limits_fit_y", QKeySequence("Shift+Ctrl+/,y"));
    auto actZoomIn = A_(tr("Zoom-in"), this, IN_ACTIVE_PLOT(zoomIn), ":/toolbar/limits_zoom_in", QKeySequence("Ctrl+Alt+="));
    auto actZoomOut = A_(tr("Zoom-out"), this, IN_ACTIVE_PLOT(zoomOut), ":/toolbar/limits_zoom_out", QKeySequence("Ctrl+Alt+-"));
    auto actZoomInX = A_(tr("Zoom-in X"), this, IN_ACTIVE_PLOT(zoomInX), ":/toolbar/limits_zoom_in_x", QKeySequence("Alt+="));
    auto actZoomInY = A_(tr("Zoom-in Y"), this, IN_ACTIVE_PLOT(zoomInY), ":/toolbar/limits_zoom_in_y", QKeySequence("Ctrl+="));
    auto actZoomOutX = A_(tr("Zoom-out X"), this, IN_ACTIVE_PLOT(zoomOutX), ":/toolbar/limits_zoom_out_x", QKeySequence("Alt+-"));
    auto actZoomOutY = A_(tr("Zoom-out Y"), this, IN_ACTIVE_PLOT(zoomOutY), ":/toolbar/limits_zoom_out_y", QKeySequence("Ctrl+-"));

    menuBar->addMenu(Ori::Gui::menu(tr("Limits"), this, {
        actLimitsBoth, actAutolimits, /*actFitSelection,*/ actZoomIn, actZoomOut, 0,
        actLimitsX, actAutolimitsX, /*actFitSelectionX,*/ actZoomInX, actZoomOutX, 0,
        actLimitsY, actAutolimitsY, /*actFitSelectionY,*/ actZoomInY, actZoomOutY,
    }));

    addToolBar(Qt::RightToolBarArea, Ori::Gui::toolbar(tr("Limits"), "limits", {
        actLimitsBoth, actAutolimits, /*actFitSelection,*/ actZoomIn, actZoomOut, 0,
        actLimitsX, actAutolimitsX, /*actFitSelectionX,*/ actZoomInX, actZoomOutX, 0,
        actLimitsY, actAutolimitsY, /*actFitSelectionY,*/ actZoomInY, actZoomOutY,
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

    _mdiToolbar = new Ori::Widgets::MdiToolBar(tr("Windows"), _mdiArea);
    _mdiToolbar->setMovable(false);
    _mdiToolbar->setFloatable(false);
    _mdiToolbar->flat = true;
    _mdiToolbar->menuForButton = Ori::Gui::menu({
        A_(tr("Rename..."), this, SLOT(renameDiagramFromMdiToolbar()), ":/toolbar/plot_rename"),
        0,
        A_(tr("Close"), this, [this]{ _mdiToolbar->windowUnderMenu->close(); }, ":/toolbar/plot_delete"),
    });
    _mdiToolbar->menuForSpace = Ori::Gui::menu({
        A_(tr("New Diagram"), this, SLOT(newPlot()), ":/toolbar/plot_new"),
    });
    addToolBar(Qt::BottomToolBarArea, _mdiToolbar);

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
    _statusBar->connect(STATUS_FACTOR_X, &Ori::Widgets::Label::doubleClicked, IN_ACTIVE_PLOT(axisFactorDlgX));
    _statusBar->connect(STATUS_FACTOR_Y, &Ori::Widgets::Label::doubleClicked, IN_ACTIVE_PLOT(axisFactorDlgY));

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
    {
        QString plotId = params.value("id").toString();
        if (_panelDataGrid->isVisible() && _panelDataGrid->plotId() == plotId)
            _panelDataGrid->showData(findPlotById(plotId), nullptr);
        break;
    }
    case MSG_GRAPH_RENAMED:
    {
        QString graphId = params.value("id").toString();
        if (_panelDataGrid->isVisible() && _panelDataGrid->graphId() == graphId)
            _panelDataGrid->showData(nullptr, findGraphById(graphId));
        break;
    }
    case MSG_GRAPH_DELETED:
    case MSG_AXIS_FACTOR_CHANGED:
    case MSG_PLOT_DELETED:
        updateStatusBar();
        break;
    }
}

void MainWindow::updateStatusBar()
{
    _statusBar->setText(STATUS_PLOTS, tr("Diagrams: %1").arg(_mdiArea->subWindowList().size()));
    if (auto plot = activePlot(false); plot) {
        _statusBar->setText(STATUS_GRAPHS, tr("Graphs: %1").arg(plot->graphCount()));
        _statusBar->setText(STATUS_FACTOR_X, plot->displayFactorX());
        _statusBar->setText(STATUS_FACTOR_Y, plot->displayFactorY());
    } else {
        _statusBar->clear(STATUS_GRAPHS);
        _statusBar->clear(STATUS_FACTOR_X);
        _statusBar->clear(STATUS_FACTOR_Y);
    }
    if (auto graph = selectedGraph(false); graph) {
        _statusBar->setText(STATUS_POINTS, tr("Points: %1").arg(graph->pointsCount()));
        _statusBar->setText(STATUS_DATA_SOURCE, graph->dataSource()->displayStr());
    } else {
        _statusBar->clear(STATUS_POINTS);
        _statusBar->clear(STATUS_DATA_SOURCE);
    }
}

void MainWindow::updateDataGrid()
{
    if (_panelDataGrid->isVisible())
        if (auto plot = qobject_cast<PlotWindow*>(sender()); plot)
            if (auto graph = selectedGraph(false); graph)
                _panelDataGrid->showData(plot->plotObj(), graph);
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

Graph* MainWindow::findGraphById(const QString& id) const
{
    foreach (auto wnd, _mdiArea->subWindowList())
    {
        auto plotWnd = qobject_cast<PlotWindow*>(wnd->widget());
        if (!plotWnd) continue;
        if (auto g = plotWnd->findGraphById(id); g)
            return g;
    }
    return nullptr;
}

Graph* MainWindow::selectedGraph(bool warn) const
{
    auto plot = activePlot(warn);
    return plot ? plot->selectedGraph(warn) : nullptr;
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

void MainWindow::deletePlot()
{
    auto plot = activePlot();
    if (plot)
        _mdiArea->currentSubWindow()->close();
}

void MainWindow::graphCreated(Graph* graph)
{
    auto plot = activePlot(false);
    if (!plot)
    {
        newPlot();
        plot = activePlot(false);
        if (!plot)
        {
            qWarning() << "Failed to create new plot window";
            delete graph;
            return;
        }
    }

    plot->addGraph(graph);

    if (AppSettings::instance().autolimitAfterGraphGreated)
        plot->autolimits();
    if (AppSettings::instance().selectNewGraph)
        plot->selectGraph(graph);
    else
        updateStatusBar();
}

void MainWindow::graphUpdated(Graph* graph)
{
    foreach (auto wnd, _mdiArea->subWindowList())
    {
        auto plotWnd = qobject_cast<PlotWindow*>(wnd->widget());
        if (plotWnd && plotWnd->updateGraph(graph))
        {
            updateStatusBar();
            updateDataGrid();
            return;
        }
    }
}

void MainWindow::graphSelected(Graph*)
{
    updateStatusBar();
    updateDataGrid();
}

void MainWindow::mdiSubWindowActivated(QMdiSubWindow*)
{
    updateStatusBar();
    updateDataGrid();
}

void MainWindow::viewMenuShown()
{
    auto plot = activePlot(false);
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

    auto graph = plot->selectedGraph(false);
    if (!graph)
    {
        // There will be popup message after copying image
        plot->copyPlotImage();
        return;
    }

    // TODO: show popup message if there is no dialog here, otherwise leave a comment
    DataExporters::copyToClipboard(graph->data());
}

void MainWindow::renameDiagramFromMdiToolbar()
{
    auto plot = qobject_cast<PlotWindow*>(_mdiToolbar->windowUnderMenu->widget());
    if (plot) plot->rename();
}
