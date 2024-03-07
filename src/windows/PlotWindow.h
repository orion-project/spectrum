#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMdiSubWindow>

#include "../app/AppSettings.h"

namespace QCPL {
//class Cursor;
//class CursorPanel;
class Plot;
}
class Graph;
class Operations;
class PlotObj;
class QCPAxis;
class QCPGraph;

class PlotItem
{
public:
    ~PlotItem();

    Graph* graph;
    QCPGraph* line;
};

class PlotWindow : public QWidget, public IAppSettingsListener
{
    Q_OBJECT

public:
    explicit PlotWindow(Operations *operations, QWidget *parent = nullptr);
    ~PlotWindow() override;

    PlotObj* plotObj() const { return _plotObj; }

    // Implements IAppSettingsListener
    void settingsChanged() override;

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
    void rename();
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
    PlotObj* _plotObj;
    QCPL::Plot* _plot;
    //QCPL::Cursor* _cursor;
    //QCPL::CursorPanel* _cursorPanel;
    QList<PlotItem*> _items;
    Operations* _operations;

    PlotItem* itemForLine(QCPGraph* line) const;
    PlotItem* itemForGraph(Graph* graph) const;

    void createContextMenus();
    void markModified(const QString& reason);
    void updateTitle(const QString& title);
    void deleteGraphs(const QVector<Graph*>& graphs);
    void addAxisVars(QCPAxis* axis);
};

#endif // PLOTWINDOW_H
