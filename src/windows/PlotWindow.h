#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMdiSubWindow>

namespace QCPL {
class Cursor;
class CursorPanel;
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


class PlotWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PlotWindow(Operations *operations, QWidget *parent = nullptr);
    ~PlotWindow() override;

    PlotObj* plotObj() const { return _plotObj; }

    int graphCount() const;
    void addGraph(Graph* g);

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
    void formatX();
    void formatY();
    void formatLegend();
    void formatGraph();
    void copyPlotImage();
    void copyPlotFormat();
    void pastePlotFormat();
    void pasteTitleFormat();
    void pasteLegendFormat();
    void pasteAxisFormat(QCPAxis *axis);
    void savePlotFormat();
    void loadPlotFormat();

    Graph* selectedGraph() const;
    void selectGraph(Graph* graph);
    bool updateGraph(Graph* graph);

    QSize sizeHint() const override { return QSize(600, 400); }

    bool isLegendVisible() const;
    void setLegendVisible(bool on);

    bool isTitleVisible() const;
    void setTitleVisible(bool on);
    void editTitle();

protected:
    void closeEvent(class QCloseEvent*) override;

signals:
    void graphSelected(Graph* g);

private slots:
    void graphLineSelected(bool selected);

private:
    PlotObj* _plotObj;
    QCPL::Plot* _plot;
    QCPL::Cursor* _cursor;
    QCPL::CursorPanel* _cursorPanel;
    QList<PlotItem*> _items;
    Operations* _operations;

    PlotItem* itemForLine(QCPGraph* line) const;
    PlotItem* itemForGraph(Graph* graph) const;

    void createContextMenus();
    void markModified(const QString& reason);
};

#endif // PLOTWINDOW_H