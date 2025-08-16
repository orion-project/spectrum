#include "MainWindow.h"

#include "Operations.h"
#include "app/AppSettings.h"
#include "app/HelpSystem.h"
#include "core/DataExporters.h"
#include "core/DataSources.h"
#include "core/Project.h"
#include "widgets/DataGridPanel.h"
#include "windows/PlotWindow.h"

#include "helpers/OriDialogs.h"
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
#include <QCloseEvent>
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

MainWindow::MainWindow(const QString &fileName, QWidget *parent)
    : QMainWindow(parent), Ori::IMessageBusListener()
{
    setObjectName("mainWindow");
    Ori::Wnd::setWindowIcon(this, ":/window_icons/main");
    
    _project = new Project(this);

    _panelDataGrid = new DataGridPanel(_project, this);

    _operations = new Operations(_project, this);
    _operations->getSelectedGraph = [this](){ return selectedGraph(); };
    _operations->getFormats = [this]{ return getFormats(); };
    connect(_operations, &Operations::graphCreated, this, &MainWindow::graphCreated);
    connect(_operations->mruPlotFormats(), &Ori::MruFileList::clicked, this, [this](const QString& fileName){
        auto plot = activePlot(); if (plot) plot->loadPlotFormat(fileName);
    });
    connect(_operations->mruProjects(), &Ori::MruFileList::clicked, this, [this](const QString& fileName){
        _operations->openPrjFile(fileName);
    });

    _mdiArea = new QMdiArea;
    _mdiArea->setBackground(QBrush(QPixmap(":/misc/mdi_background")));
    connect(_mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::mdiSubWindowActivated);

    setCentralWidget(_mdiArea);

    createDocks();
    createActions();
    createStatusBar();

    restoreState();

    QTimer::singleShot(200, this, [this, fileName](){
        if (fileName.isEmpty()) {
            _project->newDiagram();
            _project->markUnmodified("MainWindow::MainWindow");
        } else {
            _operations->openPrjFile(fileName);
        }
    });
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
    _operations->restoreState(s);
    s.restoreWindowGeometry(this);
    s.restoreDockState(this);
}

void MainWindow::createActions()
{
#define A0_ Ori::Gui::V0::action
#define A1_ Ori::Gui::action

    Ori::Gui::setActionTooltipFormat("<p style='white-space:pre'>%1</p><p><b>%2</b></p>");

    auto menuBar = this->menuBar();

    //---------------------------------------------------------

    auto actPrjNew = A1_(tr("New Project"), _operations, &Operations::prjNew, ":/toolbar/page", QKeySequence("Ctrl+N"));
    auto actPrjOpen = A1_(tr("Open Project..."), _operations, &Operations::prjOpen, ":/toolbar/open", QKeySequence("Ctrl+O"));
    auto actPrjSave = A1_(tr("Save Project"), _operations, &Operations::prjSave, ":/toolbar/save", QKeySequence("Ctrl+S"));
    auto actPrjSaveAs = A1_(tr("Save Project As..."), _operations, &Operations::prjSaveAs);
    auto actPlotNew = A1_(tr("New Diagram"), _project, &Project::newDiagram, ":/toolbar/plot_new", QKeySequence("Shift_Ctrl+N"));
    auto actPlotRename = A1_(tr("Rename Diagram..."), this, IN_ACTIVE_PLOT(renamePlot), ":/toolbar/plot_rename", QKeySequence("Ctrl+F2"));
    auto actPlotDelete = A1_(tr("Delete Diagram"), this, &MainWindow::deletePlot, ":/toolbar/plot_delete");
    auto actPlotSaveImg = A1_(tr("Save Diagram as Image..."), this, IN_ACTIVE_PLOT(exportPlotImg), ":/toolbar/save_img");
    auto actPlotSavePrj = A1_(tr("Save Diagram as Project..."), this, IN_ACTIVE_PLOT(exportPlotPrj), ":/toolbar/plot_save");
    auto actExit = A0_(tr("Exit"), this, SLOT(close()));

    auto menuPrj = Ori::Gui::menu(tr("Project"), this, {
        actPrjNew, actPrjOpen, actPrjSave, actPrjSaveAs, 0,
        actPlotNew, actPlotRename, actPlotDelete, actPlotSavePrj, actPlotSaveImg, 0,
        actExit
    });
    menuBar->addMenu(menuPrj);
    
    new Ori::Widgets::MruMenuPart(_operations->mruProjects(), menuPrj, actExit, this);

    addToolBar(Ori::Gui::toolbar(tr("Project"), "project", {
        actPrjNew, actPrjOpen, actPrjSave, 0,
        actPlotNew, actPlotRename, actPlotDelete, actPlotSavePrj, actPlotSaveImg,
    }));

    //---------------------------------------------------------

    auto actCopy = A0_(tr("Copy"), tr("Copy graph data or image"), this, SLOT(editCopy()), ":/toolbar/copy", QKeySequence::Copy);
    auto actPaste = A0_(tr("Paste"), tr("Create graph from Clipboard data"), _operations, SLOT(addFromClipboard()), ":/toolbar/paste", QKeySequence::Paste);
    auto actPasteCsv = A0_(tr("Paste as CSV..."), tr("Create graphs from Clipboard data (with config dialog)"), _operations, SLOT(addFromClipboardCsv()), ":/toolbar/paste_table", QKeySequence("Ctrl+Alt+V"));
    auto actCopyFormat = A1_(tr("Copy Format"), tr("Copy diagram format"), this, IN_ACTIVE_PLOT(copyPlotFormat), ":/toolbar/copy_fmt", QKeySequence("Ctrl+Shift+C"));
    auto actPasteFormat = A1_(tr("Paste Format"), tr("Paste diagram format"), this, IN_ACTIVE_PLOT(pastePlotFormat), ":/toolbar/paste_fmt", QKeySequence("Ctrl+Shift+V"));

    menuBar->addMenu(Ori::Gui::menu(tr("Edit"), this, {
        actCopy, actPaste, actPasteCsv, 0, actCopyFormat, actPasteFormat,
    }));

    addToolBar(Ori::Gui::toolbar(tr("Edit"), "edit", {
        actCopy, actPaste, actPasteCsv
    }));

    //---------------------------------------------------------

    _actToggleDatagrid = A0_(tr("Data Grid"), tr("Toggle data grid panel"), this, SLOT(toggleDataGrid()), ":/toolbar/panel_datagrid");
    _actViewTitle = A1_(tr("Title"), tr("Toggle diagram title"), this, IN_ACTIVE_PLOT(toggleTitle), ":/toolbar/plot_title");
    _actViewLegend = A1_(tr("Legend"), tr("Toggle diagram legend"), this, IN_ACTIVE_PLOT(toggleLegend), ":/toolbar/plot_legend");
    _actToggleDatagrid->setCheckable(true);
    _actViewTitle->setCheckable(true);
    _actViewLegend->setCheckable(true);

    auto menuView = Ori::Gui::menu(tr("View"), this, {
        _actToggleDatagrid, 0, _actViewTitle, _actViewLegend,
    });
    connect(menuView, &QMenu::aboutToShow, this, &MainWindow::viewMenuShown);
    menuBar->addMenu(menuView);

    //---------------------------------------------------------

    auto actAddFile = A0_(tr("From File..."), tr("Create graph from file data"), _operations, SLOT(addFromFile()), ":/toolbar/add_file", Qt::Key_Insert);
    auto actAddCsv = A0_(tr("From CSV File..."), tr("Create graphs from file (with config dialog)"), _operations, SLOT(addFromCsvFile()), ":/toolbar/add_table", QKeySequence("Shift+Ctrl+Ins"));
    auto actAddClipboard = A0_(tr("From Clipboard"), tr("Create graph from Clipboard data"), _operations, SLOT(addFromClipboard()), ":/toolbar/paste");
    auto actAddCsvClipboard = A0_(tr("From Clipboard as CSV..."), tr("Create graphs from Clipboard data (with config dialog)"), _operations, SLOT(addFromClipboardCsv()), ":/toolbar/paste_table");
    auto actAddRandom = A0_(tr("Random Sample..."), tr("Add random sample graph"), _operations, SLOT(addRandomSample()), ":/toolbar/add_random");

    menuBar->addMenu(Ori::Gui::menu(tr("Add"), this, {
        actAddFile, actAddCsv, 0, actAddClipboard, actAddCsvClipboard, 0, actAddRandom,
    }));

    addToolBar(Ori::Gui::toolbar(tr("Add"), "add", {
        actAddFile, actAddCsv, 0, actAddRandom,
    }));

    //---------------------------------------------------------

    auto actnGraphRefresh = A0_(tr("Refresh"), tr("Reread points from data source"), _operations, SLOT(graphRefresh()), ":/toolbar/update", QKeySequence("Ctrl+R"));
    auto actGraphReopen = A0_(tr("Reopen..."), tr("Reselect or reconfigure data source"), _operations, SLOT(graphReopen()), ":/toolbar/update_params");
    auto actGraphTitle = A1_(tr("Title..."), tr("Edit title of selected graph"), this, IN_ACTIVE_PLOT(renameGraph), ":/toolbar/graph_title", QKeySequence("F2"));
    auto actGraphProps = A1_(tr("Line Format..."), tr("Set line format of selected graph"), this, IN_ACTIVE_PLOT(formatGraph), ":/toolbar/graph_format");
    auto actGraphDelete = A1_(tr("Delete"), tr("Delete selected graphs"), this, IN_ACTIVE_PLOT(deleteGraph), ":/toolbar/graph_delete", QKeySequence("Del"));
    auto actGraphAxes = A1_(tr("Change Axes..."), this, IN_ACTIVE_PLOT(changeGraphAxes));

    menuBar->addMenu(Ori::Gui::menu(tr("Graph"), this, {
        actnGraphRefresh, actGraphReopen, 0, actGraphTitle, actGraphProps, actGraphAxes, 0, actGraphDelete,
    }));

    // By default the Graph toolbar is in the second row, should be added after all others
    auto tbGraph = Ori::Gui::toolbar(tr("Graph"), "graph", {
        actnGraphRefresh, actGraphReopen, 0, actGraphTitle, actGraphProps, 0, actGraphDelete,
    });

    //---------------------------------------------------------

    auto actOffset = A0_(tr("Offset (Graph ± Const)..."), _operations, SLOT(modifyOffset()), ":/toolbar/graph_offset", Qt::Key_Plus);
    // auto actFlip = A0_(tr("Flip (Const - Graph)..."), _operations, SLOT(modifyFlip()), ":/toolbar/graph_flip");
    // auto actReflect = A0_(tr("Reflect..."), _operations, SLOT(modifyReflect()), ":/toolbar/graph_reflect");
    auto actScale = A0_(tr("Scale (Graph × Const)..."), _operations, SLOT(modifyScale()), ":/toolbar/graph_scale", Qt::Key_Asterisk);
    auto actNormalize = A0_(tr("Normalize (Graph ÷ Const)..."), _operations, SLOT(modifyNormalize()), ":/toolbar/graph_norm", Qt::Key_Slash);
    auto actInvert = A0_(tr("Invert (Const ÷ Graph)..."), _operations, SLOT(modifyInvert()), ":/toolbar/graph_inv");
    auto actDecimate = A0_(tr("Decimate..."), _operations, SLOT(modifyDecimate()), ":/toolbar/graph_decim");
    // auto actAverage = A0_(tr("Average..."), _operations, SLOT(modifyAverage()), ":/toolbar/graph_avg");
    auto actMavgSimple = A0_(tr("Moving Average (simple)..."), _operations, SLOT(modifyMavgSimple()), ":/toolbar/graph_mavg");
    auto actMavgCumul = A0_(tr("Moving Average (cumulative)"), _operations, SLOT(modifyMavgCumul()));
    auto actMavgExp = A0_(tr("Moving Average (exponential)..."), _operations, SLOT(modifyMavgExp()));
    auto actFitLimits = A0_(tr("Fit Limits..."), _operations, SLOT(modifyFitLimits()), ":/toolbar/graph_fit");
    auto actDespike = A0_(tr("Remove Spikes..."), _operations, SLOT(modifyDespike()), ":/toolbar/graph_despike");
    auto actDerivatie = A0_(tr("First Derivative..."), _operations, SLOT(modifyDerivative()));

    menuBar->addMenu(Ori::Gui::menu(tr("Modify"), this, {
        actOffset,
        // actFlip, actReflect,
        0, actScale, actNormalize, actInvert, 0, actDecimate,
        // actAverage,
        0, actMavgSimple, actMavgCumul, actMavgExp,
        0, actFitLimits, 0, actDespike, 0, actDerivatie
    }));

    addToolBar(Ori::Gui::toolbar(tr("Modify"), "modify", {
        actOffset,
        // actReflect,
        0, actScale, actNormalize, actInvert, 0, actDecimate,
        // actAverage,
        0, actMavgSimple,
        0, actFitLimits, 0, actDespike
    }));

    //---------------------------------------------------------

    auto actFormatTitle = A1_(tr("Title Format..."), this, IN_ACTIVE_PLOT(formatTitle), ":/toolbar/plot_title");
    auto actFormatX = A1_(tr("Bottom Axis Format..."), this, IN_ACTIVE_PLOT(formatX), ":/toolbar/format_x");
    auto actFormatY = A1_(tr("Left Axis Format..."), this, IN_ACTIVE_PLOT(formatY), ":/toolbar/format_y");
    auto actFormatX2 = A1_(tr("Top Axis Format..."), this, IN_ACTIVE_PLOT(formatX2));
    auto actFormatY2 = A1_(tr("Right Axis Format..."), this, IN_ACTIVE_PLOT(formatY2));
    auto actFormatAxis = A1_(tr("Format Axis..."), this, IN_ACTIVE_PLOT(formatAxis), ":/toolbar/format_axis");
    auto actFactorAxis = A1_(tr("Set Axis Factor..."), this, IN_ACTIVE_PLOT(axisFactorDlg), ":/toolbar/factor_axis");
    auto actFormatLegend = A1_(tr("Legend Format..."), this, IN_ACTIVE_PLOT(formatLegend), ":/toolbar/plot_legend");
    auto actSavePlotFormat = A1_(tr("Save Plot Format..."), this, IN_ACTIVE_PLOT(savePlotFormatDlg), ":/toolbar/save_format");
    auto actLoadPlotFormat = A1_(tr("Load Plot Format..."), this, IN_ACTIVE_PLOT(loadPlotFormatDlg), ":/toolbar/open_format");

    auto menuAddAxis = Ori::Gui::menu(tr("Add Axis"), {
        A1_(tr("At Bottom"), this, IN_ACTIVE_PLOT(addAxisBottom), ":/toolbar/axis_bottom"),
        A1_(tr("At Left"), this, IN_ACTIVE_PLOT(addAxisLeft), ":/toolbar/axis_left"),
        A1_(tr("At Top"), this, IN_ACTIVE_PLOT(addAxisTop), ":/toolbar/axis_top"),
        A1_(tr("At Right"), this, IN_ACTIVE_PLOT(addAxisRight), ":/toolbar/axis_right"),
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
        0, actSavePlotFormat, actLoadPlotFormat,
    });
    addToolBar(Qt::LeftToolBarArea, tbFormat);

    //---------------------------------------------------------

    auto actLimitsBoth = A1_(tr("Limits for Both Axes..."), this, IN_ACTIVE_PLOT(limitsDlg), ":/toolbar/limits", QKeySequence("Shift+Ctrl+="));
    auto actLimitsX = A1_(tr("Limits X..."), this, IN_ACTIVE_PLOT(limitsDlgX), ":/toolbar/limits_x", QKeySequence("Shift+Ctrl+X"));
    auto actLimitsY = A1_(tr("Limits Y..."), this, IN_ACTIVE_PLOT(limitsDlgY), ":/toolbar/limits_y", QKeySequence("Shift+Ctrl+Y"));
    auto actAutolimits = A1_(tr("Autolimits"), this, IN_ACTIVE_PLOT(autolimits), ":/toolbar/limits_auto", QKeySequence("Alt+0"));
    auto actAutolimitsX = A1_(tr("Autolimits X"), this, IN_ACTIVE_PLOT(autolimitsX), ":/toolbar/limits_auto_x", QKeySequence("Alt+X"));
    auto actAutolimitsY = A1_(tr("Autolimits Y"), this, IN_ACTIVE_PLOT(autolimitsY), ":/toolbar/limits_auto_y", QKeySequence("Alt+Y"));
//    auto actFitSelection = A1_(tr("Fit Selection"), this, IN_ACTIVE_PLOT(limitsToSelection), ":/toolbar/limits_fit", QKeySequence("Ctrl+/"));
//    auto actFitSelectionX = A1_(tr("Fit Selection X"), this, IN_ACTIVE_PLOT(limitsToSelectionX), ":/toolbar/limits_fit_x", QKeySequence("Shift+Ctrl+/,x"));
//    auto actFitSelectionY = A1_(tr("Fit Selection Y"), this, IN_ACTIVE_PLOT(limitsToSelectionY), ":/toolbar/limits_fit_y", QKeySequence("Shift+Ctrl+/,y"));
    auto actZoomIn = A1_(tr("Zoom-in"), this, IN_ACTIVE_PLOT(zoomIn), ":/toolbar/limits_zoom_in", QKeySequence("Ctrl+Alt+="));
    auto actZoomOut = A1_(tr("Zoom-out"), this, IN_ACTIVE_PLOT(zoomOut), ":/toolbar/limits_zoom_out", QKeySequence("Ctrl+Alt+-"));
    auto actZoomInX = A1_(tr("Zoom-in X"), this, IN_ACTIVE_PLOT(zoomInX), ":/toolbar/limits_zoom_in_x", QKeySequence("Alt+="));
    auto actZoomInY = A1_(tr("Zoom-in Y"), this, IN_ACTIVE_PLOT(zoomInY), ":/toolbar/limits_zoom_in_y", QKeySequence("Ctrl+="));
    auto actZoomOutX = A1_(tr("Zoom-out X"), this, IN_ACTIVE_PLOT(zoomOutX), ":/toolbar/limits_zoom_out_x", QKeySequence("Alt+-"));
    auto actZoomOutY = A1_(tr("Zoom-out Y"), this, IN_ACTIVE_PLOT(zoomOutY), ":/toolbar/limits_zoom_out_y", QKeySequence("Ctrl+-"));

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

    auto actAppSettings = A1_(tr("Settings..."), this, []{ AppSettings::instance().edit(); }, ":/toolbar/settings");

    menuBar->addMenu(Ori::Gui::menu(tr("Tools"), this, {
        actAppSettings
    }));

    //---------------------------------------------------------

    auto actWndCascade = A0_(tr("Cascade"), _mdiArea, SLOT(cascadeSubWindows()), ":/toolbar/wnd_cascade");
    auto actWndTile = A0_(tr("Tile"), _mdiArea, SLOT(tileSubWindows()), ":/toolbar/wnd_tile");

    auto menuWindow = Ori::Gui::menu(tr("Windows"), this, {
        actWndCascade, actWndTile,
    });
    menuBar->addMenu(menuWindow);
    // TODO: connect(menuWindow, QMenu::aboutToShow, _mdiArea, QMdiArea::populateWindowMenu);

    //---------------------------------------------------------

    auto help = Z::HelpSystem::instance();
    auto actHelpContent = A0_(tr("Help"), help, SLOT(showContent()), ":/toolbar/help", QKeySequence::HelpContents);
    auto actHelpBugReport = A0_(tr("Send Bug Report"), help, SLOT(sendBugReport()), ":/toolbar/bug");
    auto actHelpUpdates = A0_(tr("Check for Updates"), help, SLOT(checkUpdates()), ":/toolbar/update");
    auto actHelpHomepage = A0_(tr("Visit Homepage"), help, SLOT(visitHomePage()), ":/toolbar/home");
    auto actHelpAbout = A0_(tr("About..."), help, SLOT(showAbout()), ":/window_icons/main");

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
        A0_(tr("Rename..."), this, SLOT(renameDiagramFromMdiToolbar()), ":/toolbar/plot_rename"),
        0,
        A1_(tr("Close"), this, [this]{ _mdiToolbar->windowUnderMenu->close(); }, ":/toolbar/plot_delete"),
    });
    _mdiToolbar->menuForSpace = Ori::Gui::menu({
        A1_(tr("New Diagram"), _project, &Project::newDiagram, ":/toolbar/plot_new"),
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
    connect(versionLabel, &Ori::Widgets::Label::doubleClicked, [](){ Z::HelpSystem::instance()->showAbout(); });
    _statusBar->addPermanentWidget(versionLabel);

    setStatusBar(_statusBar);
}

void MainWindow::closeEvent(QCloseEvent* ce)
{
    if (_operations->canClose())
        ce->accept();
    else
        ce->ignore();
}

void MainWindow::messageBusEvent(int event, const QMap<QString, QVariant>& params)
{
    switch (event)
    {
    case BusEvent::ProjectModified::id:
    case BusEvent::ProjectUnmodified::id:
        updateStatusBar();
        break;
    case BusEvent::DiagramAdded::id:
        handleDiagramAdded(params.value("id").toString());
        break;
    case BusEvent::ErrorMessage::id:
        Ori::Dlg::error(params.value("error").toString());
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
    if (_project->modified())
        _statusBar->setText(STATUS_MODIF, tr("Modified"));
    else _statusBar->clear(STATUS_MODIF);
    setWindowModified(_project->modified());
    Ori::Wnd::setWindowFilePath(this, _project->fileName());
}

void MainWindow::updateDataGrid()
{
    if (_panelDataGrid->isVisible())
        if (auto plot = activePlot(); plot)
            if (auto graph = selectedGraph(false); graph)
                _panelDataGrid->showData(plot->diagram(), graph);
}

PlotWindow* MainWindow::activePlot(bool warn) const
{
    auto mdiChild = _mdiArea->currentSubWindow();
    auto plotWnd = mdiChild ? dynamic_cast<PlotWindow*>(mdiChild->widget()) : nullptr;
    if (!plotWnd and warn)
        PopupMessage::warning(tr("There is no active diagram"));
    return plotWnd;
}

Graph* MainWindow::selectedGraph(bool warn) const
{
    auto plot = activePlot(warn);
    return plot ? plot->selectedGraph(warn) : nullptr;
}

void MainWindow::handleDiagramAdded(const QString& id)
{
    auto dia = _project->diagram(id);
    if (!dia) {
        qWarning() << "MainWindow::handleDiagramAdded: Diagram not found" << id;
        return;
    }
    auto plotWindow = new PlotWindow(_operations, dia);
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
        _project->newDiagram();
        plot = activePlot(false);
        if (!plot)
        {
            qWarning() << "Failed to create new plot window";
            delete graph;
            return;
        }
    }
    plot->diagram()->addGraph(graph);
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
    if (plot) plot->renamePlot();
}

QHash<const void*, QJsonObject> MainWindow::getFormats() const
{
    QHash<const void*, QJsonObject> formats;
    auto mdiChildren = _mdiArea->subWindowList();
    for (auto mdiChild : std::as_const(mdiChildren)) {
        auto plotWnd = dynamic_cast<PlotWindow*>(mdiChild->widget());
        if (!plotWnd) continue;
        auto plotFormats = plotWnd->getFormats();
        for (auto it = plotFormats.cbegin(); it != plotFormats.cend(); it++)
            formats.insert(it.key(), it.value());
    }
    return formats;
}
