#ifndef PLOT_WINDOW_H
#define PLOT_WINDOW_H

#include "app/AppSettings.h"

#include "tools/OriMessageBus.h"

#include <QMdiSubWindow>

namespace QCPL {
//class Cursor;
//class CursorPanel;
class Plot;
}
class Diagram;
class Graph;
class Operations;
class Project;
class QCPAxis;
class QCPGraph;

class PlotItem
{
public:
    ~PlotItem();

    Graph* graph;
    QCPGraph* line;
};

class PlotWindow : public QWidget, public IAppSettingsListener, public Ori::IMessageBusListener
{
    Q_OBJECT

public:
    explicit PlotWindow(Operations *operations, Diagram *_diagram, QWidget *parent = nullptr);
    ~PlotWindow() override;

    Diagram* diagram() const { return _diagram; }

    // Implements IAppSettingsListener
    void settingsChanged() override;

    // Ori::IMessageBusListener
    void messageBusEvent(int event, const QMap<QString, QVariant>& params) override;

    int graphCount() const;
    void addGraph(Graph* g);

    void limitsDlg();
    void limitsDlgX();
    void limitsDlgY();
    void autolimits();
    void autolimitsX();
    void autolimitsY();
//    void limitsToSelection();
//    void limitsToSelectionX();
//    void limitsToSelectionY();
    void zoomIn();
    void zoomOut();
    void zoomInX();
    void zoomOutX();
    void zoomInY();
    void zoomOutY();
    void formatX();
    void formatY();
    void formatX2();
    void formatY2();
    void formatAxis();
    void formatTitle();
    void formatLegend();
    void formatGraph();
    void addAxisBottom();
    void addAxisLeft();
    void addAxisTop();
    void addAxisRight();
    void copyPlotImage();
    void copyPlotFormat();
    void pastePlotFormat();
    void pasteTitleFormat();
    void pasteLegendFormat();
    void pasteAxisFormat(QCPAxis *axis);
    void savePlotFormatDlg();
    void loadPlotFormatDlg();
    void loadPlotFormat(const QString& fileName);
    void renameGraph();
    void deleteGraph();
    void toggleLegend();
    void toggleTitle();
    void axisFactorDlgX();
    void axisFactorDlgY();
    void axisFactorDlg();
    void copyGraphFormat();
    void pasteGraphFormat();
    void exportPlotImg();
    void exportPlotPrj();
    void changeGraphAxes();

    Graph* selectedGraph(bool warn = true) const;
    QVector<Graph*> selectedGraphs(bool warn = true) const;
    QCPGraph* selectedGraphLine(bool warn = true) const;
    Graph* findGraphById(const QString& id) const;
    void selectGraph(Graph* graph);
    void selectGraphLine(QCPGraph* line, bool replot = true);
    bool updateGraph(Graph* graph);

    QSize sizeHint() const override { return QSize(600, 400); }

    bool isLegendVisible() const;
    bool isTitleVisible() const;
    QString displayFactorX() const;
    QString displayFactorY() const;

protected:
    void closeEvent(class QCloseEvent*) override;

signals:
    void graphSelected(Graph* g);

private slots:
    void graphLineSelected(bool selected);

private:
    Diagram* _diagram;
    QCPL::Plot* _plot;
    //QCPL::Cursor* _cursor;
    //QCPL::CursorPanel* _cursorPanel;
    QList<PlotItem*> _items;
    Operations *_operations;

    PlotItem* itemForLine(QCPGraph* line) const;
    PlotItem* itemForGraph(Graph* graph) const;

    void createContextMenus();
    void updateTitle();
    void deleteGraphs(const QVector<Graph*>& graphs);
    void addAxisVars(QCPAxis* axis);
    
};

#endif // PLOT_WINDOW_H
