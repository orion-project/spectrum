#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

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
class Project;

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

protected:
    void closeEvent(class QCloseEvent*) override;
    
private:
    QMdiArea* _mdiArea;
    Project* _project;
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
    Graph* selectedGraph(bool warn = true) const;
    void graphSelected(Graph* graph);
    void graphCreated(Graph* graph);
    void mdiSubWindowActivated(QMdiSubWindow* window);
    void viewMenuShown();
    void updateDataGrid();
    void updateStatusBar();

    void deletePlot();
    
    void handleDiagramAdded(const QString& id);

private slots:
    void editCopy();
    void toggleDataGrid();
    void renameDiagramFromMdiToolbar();
};

#endif // MAIN_WINDOW_H
