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
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void graphCreated(Graph* graph) const;

private:
    QMdiArea* _mdiArea;
    Operations* _operations;
    QDockWidget *_dockDataGrid;
    DataGridPanel *_panelDataGrid;
    QToolBar *_toolbarMdi;

    void createMenu();
    void createDocks();
    void createToolBars();
    void createStatusBar();
    void saveSettings();
    void loadSettings();
    void newProject();
    void newPlot();

    PlotWindow* activePlot() const;
    void graphSelected(Graph* graph);
    void mdiSubWindowActivated(QMdiSubWindow *window);
};

#endif // MAINWINDOW_H
