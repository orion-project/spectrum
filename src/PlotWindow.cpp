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
    // TODO set auto line color
    _plot->replot();
}

void PlotWindow::autolimits()
{
    _plot->autolimits();
}
