#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "tools/OriMessageBus.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QDockWidget;
class QMdiArea;
class QMdiSubWindow;
QT_END_NAMESPACE

class Graph;
class PlotObj;
class DataGridPanel;
class Operations;
class PlotWindow;

namespace Ori::Widgets {
class StatusBar;
class MdiToolBar;
}

class MainWindow : public QMainWindow, public Ori::IMessageBusListener
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Ori::IMessageBusListener
    void messageBusEvent(int event, const QMap<QString, QVariant>& params) override;

private:
    QMdiArea* _mdiArea;
    Operations* _operations;
    QDockWidget *_dockDataGrid;
    DataGridPanel *_panelDataGrid;
    Ori::Widgets::StatusBar *_statusBar;
    Ori::Widgets::MdiToolBar *_mdiToolbar;
    QAction *_actToggleDatagrid, *_actViewTitle, *_actViewLegend;

    void createActions();
    void createDocks();
    void createStatusBar();
    void storeState();
    void restoreState();

    PlotWindow* activePlot(bool warn = true) const;
    PlotObj* findPlotById(const QString& id) const;
    Graph* findGraphById(const QString& id) const;
    Graph* selectedGraph(bool warn = true) const;
    void graphSelected(Graph* graph);
    void graphCreated(Graph* graph);
    void graphUpdated(Graph* graph);
    void mdiSubWindowActivated(QMdiSubWindow* window);
    void viewMenuShown();
    void updateDataGrid();
    void updateStatusBar();

private slots:
    void newPlot();
    void deletePlot();
    void editCopy();
    void toggleDataGrid();
    void renameDiagramFromMdiToolbar();
};

#endif // MAINWINDOW_H
