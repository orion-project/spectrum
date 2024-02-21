#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QDockWidget;
class QMdiArea;
class QMdiSubWindow;
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
    Ori::Widgets::StatusBar *_statusBar;

    void createActions();
    void createDocks();
    void createStatusBar();
    void saveSettings();
    void loadSettings();
    void newProject();

    PlotWindow* activePlot() const;
    Graph* selectedGraph() const;
    void graphSelected(Graph* graph) const;
    void graphCreated(Graph* graph) const;
    void graphUpdated(Graph* graph) const;
    void mdiSubWindowActivated(QMdiSubWindow *window) const;

private slots:
    void newPlot();
    void renamePlot();
    void deletePlot();
    void editCopy();
    void editPaste();
    void limitsDlg();
    void limitsDlgX();
    void limitsDlgY();
    void autolimits();
    void autolimitsX();
    void autolimitsY();
    void limitsToSelection();
    void limitsToSelectionX();
    void limitsToSelectionY();
    void zoomIn();
    void zoomOut();
    void zoomInX();
    void zoomOutX();
    void zoomInY();
    void zoomOutY();
    void toggleLegend();
    void toggleTitle();
    void toggleDataGrid();
    void formatTitle();
    void formatX();
    void formatY();
    void formatLegend();
    void formatGraph();
};

#endif // MAINWINDOW_H