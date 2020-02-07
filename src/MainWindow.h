#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QDockWidget;
class QMdiArea;
class QMdiSubWindow;
class QToolBar;
QT_END_NAMESPACE

class Graph;
class DataGridPanel;
class Operations;
class PlotWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

private:
    QMdiArea* _mdiArea;
    Operations* _operations;
    QDockWidget *_dockDataGrid;
    DataGridPanel *_panelDataGrid;
    QToolBar *_toolbarMdi, *_toolbarProject, *_toolbarGraph, *_toolbarLimits;
    QAction *_projNewPlot;
    QAction *_viewTitle, *_viewLegend;
    QAction *_actnMakeRandomSample, *_actnMakeRandomSampleParams, *_actnMakeFromClipboard, *_actnMakeFromFile;
    QAction *_actnGraphRefresh;
    QAction *_actnModifyOffset;
    QAction *_limitsAuto;

    void createMenu();
    void createDocks();
    void createToolBars();
    void createStatusBar();
    void fillToolbars();
    void saveSettings();
    void loadSettings();
    void newProject();
    void newPlot();
    void autolimits();
    void updateViewMenu();
    void toggleLegend();
    void toggleTitle();

    PlotWindow* activePlot() const;
    Graph* selectedGraph() const;
    void graphSelected(Graph* graph) const;
    void graphCreated(Graph* graph) const;
    void graphUpdated(Graph* graph) const;
    void mdiSubWindowActivated(QMdiSubWindow *window) const;
};

#endif // MAINWINDOW_H
