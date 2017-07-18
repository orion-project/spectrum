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

    void addGraph(Graph* g);

    void autolimits();

    QSize sizeHint() const override { return QSize(600, 400); }

private:
    QCPL::Plot* _plot;

    QList<PlotItem*> _items;
};

#endif // PLOTWINDOW_H
