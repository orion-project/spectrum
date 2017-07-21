#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QWidget>

namespace QCPL {
class Plot;
}

class Graph;
class PlotObj;
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
    explicit PlotWindow(QWidget *parent = nullptr);
    ~PlotWindow();

    PlotObj* plotObj() const { return _plotObj; }

    void addGraph(Graph* g);

    void autolimits();

    Graph* selectedGraph() const;
    void selectGraph(Graph* graph);

    QSize sizeHint() const override { return QSize(600, 400); }

    bool isLegendVisible() const;
    void setLegendVisible(bool on);

signals:
    void graphSelected(Graph* g);

private slots:
    void graphLineSelected(bool selected);

private:
    PlotObj* _plotObj;
    QCPL::Plot* _plot;
    QList<PlotItem*> _items;

    PlotItem* itemForLine(QCPGraph* line) const;
    PlotItem* itemForGraph(Graph* graph) const;
};

#endif // PLOTWINDOW_H
