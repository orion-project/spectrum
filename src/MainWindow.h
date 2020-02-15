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

namespace Ori {
namespace Widgets {
    class StatusBar;
}}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QMdiArea* _mdiArea;
    Operations* _operations;
    QDockWidget *_dockDataGrid;
    DataGridPanel *_panelDataGrid;
    QAction *_actnPlotNew;
    QAction *_actnViewTitle, *_actnViewLegend;
    QAction *_actnAddRandomSample, *_actnAddFromClipboard, *_actnAddFromFile;
    QAction *_actnGraphRefresh;
    QAction *_actnModifyOffset;
    QAction *_actnLimitsAuto;
    Ori::Widgets::StatusBar *_statusBar;

    void createActions();
    void createDocks();
    void createStatusBar();
    void saveSettings();
    void loadSettings();
    void newProject();
    void newPlot();
    void autolimits();
    void updateViewMenu();
    void toggleLegend();
    void toggleTitle();
    void editCopy();
    void editPaste();

    PlotWindow* activePlot() const;
    Graph* selectedGraph() const;
    void graphSelected(Graph* graph) const;
    void graphCreated(Graph* graph) const;
    void graphUpdated(Graph* graph) const;
    void mdiSubWindowActivated(QMdiSubWindow *window) const;
};

#endif // MAINWINDOW_H
