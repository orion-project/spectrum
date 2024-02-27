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

namespace Ori {
namespace Widgets {
    class StatusBar;
}}

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
    QAction *_actToggleDatagrid, *_actViewTitle, *_actViewLegend;

    void createActions();
    void createDocks();
    void createStatusBar();
    void saveSettings();
    void loadSettings();
    void newProject();

    PlotWindow* activePlot(bool warn = true) const;
    PlotObj* findPlotById(const QString& id) const;
    Graph* findGraphById(const QString& id) const;
    Graph* selectedGraph() const;
    QVector<Graph*> selectedGraphs() const;
    void graphSelected(Graph* graph);
    void graphCreated(Graph* graph);
    void graphUpdated(Graph* graph);
    void mdiSubWindowActivated(QMdiSubWindow* window);
    void viewMenuShown();
    void updateDataGrid();
    void updateStatusBar();

private slots:
    void newPlot();
    void renamePlot();
    void deletePlot();
    void editCopy();
    void editPaste();
    void toggleDataGrid();
};

#endif // MAINWINDOW_H
