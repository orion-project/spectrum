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
    void selectGraph(Graph* graph);

    QSize sizeHint() const override { return QSize(600, 400); }

    bool isLegendVisible() const;
    void setLegendVisible(bool on);

signals:
    void graphSelected(Graph* g);

private slots:
    void graphLineSelected(bool selected);

private:
    QCPL::Plot* _plot;
    int _nextColorIndex = 0;
    QList<PlotItem*> _items;

    QColor nextGraphColor();

    PlotItem* itemForLine(QCPGraph* line) const;
    PlotItem* itemForGraph(Graph* graph) const;
};

#endif // PLOTWINDOW_H
