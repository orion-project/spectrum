#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QDockWidget;
class QMdiArea;
class QMdiSubWindow;
class QToolBar;
class QTabWidget;
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
    QAction *_actnViewTitle, *_actnViewLegend;
    Ori::Widgets::StatusBar *_statusBar;
    QTabWidget *_toolTabs;

    void createActions();
    void createDocks();
    void createStatusBar();
    void createTools(QString title, std::initializer_list<QObject *> items);
    void saveSettings();
    void loadSettings();
    void newProject();
    void updateViewMenu();
    void toggleLegend();
    void toggleTitle();

    PlotWindow* activePlot() const;
    Graph* selectedGraph() const;
    void graphSelected(Graph* graph) const;
    void graphCreated(Graph* graph) const;
    void graphUpdated(Graph* graph) const;
    void mdiSubWindowActivated(QMdiSubWindow *window) const;

private slots:
    void newPlot();
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
};

#endif // MAINWINDOW_H
