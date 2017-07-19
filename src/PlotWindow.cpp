#include "PlotWindow.h"
#include "qcpl_plot.h"
#include "core/Graph.h"
#include "helpers/OriLayouts.h"

PlotItem::~PlotItem()
{
    delete graph;
}

//------------------------------------------------------------------------------

PlotWindow::PlotWindow(QWidget *parent) : QWidget(parent)
{
    _plot = new QCPL::Plot;
    connect(_plot, &QCPL::Plot::graphSelected, this, &PlotWindow::graphLineSelected);

    Ori::Layouts::LayoutV({_plot}).setMargin(0).setSpacing(0).useFor(this);
}

PlotWindow::~PlotWindow()
{
    qDeleteAll(_items);
}

void PlotWindow::addGraph(Graph* g)
{
    auto item = new PlotItem;
    item->graph = g;
    item->line = _plot->addGraph();
    item->line->setData(g->x(), g->y());
    _items.append(item);
    // TODO set auto line color
    _plot->replot();
}

void PlotWindow::autolimits()
{
    _plot->autolimits();
}

void PlotWindow::graphLineSelected(QCPGraph* g)
{
    auto item = itemForGraph(g);
    if (item)
        emit graphSelected(item->graph);
}

PlotItem* PlotWindow::itemForGraph(QCPGraph* g)
{
    for (auto item : _items)
        if (item->line == g)
            return item;
    return nullptr;
}
