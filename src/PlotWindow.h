#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QWidget>

namespace QCPL {
class Plot;
}

class Graph;
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

    QString plotTitle() const { return windowTitle(); }

    void addGraph(Graph* g);

    void autolimits();

    Graph* selectedGraph() const;

    QSize sizeHint() const override { return QSize(600, 400); }

signals:
    void graphSelected(Graph* g);

private:
    QCPL::Plot* _plot;

    QList<PlotItem*> _items;

    void graphLineSelected(QCPGraph* g);

    PlotItem* itemForGraphLine(QCPGraph* g) const;
};

#endif // PLOTWINDOW_H
