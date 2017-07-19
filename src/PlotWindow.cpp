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
    setWindowIcon(QIcon(":/icon/ball")); // TODO

    _plot = new QCPL::Plot;
    //connect(_plot, &QCPL::Plot::graphSelected, this, &PlotWindow::graphLineSelected);

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
    item->line->setName(g->title());
    item->line->setSelectable(QCP::stWhole);
    item->line->setData(g->x(), g->y());
    connect(item->line, SIGNAL(selectionChanged(bool)), this, SLOT(graphLineSelected(bool)));
    _items.append(item);
    // TODO set auto line color
    _plot->replot();
}

void PlotWindow::autolimits()
{
    _plot->autolimits();
}

//void PlotWindow::graphLineSelected(QCPGraph* g)
//{
//    qDebug() << "selected";
//    auto item = itemForLine(g);
//    if (item)
//        emit graphSelected(item->graph);
//}

void PlotWindow::graphLineSelected(bool selected)
{
    auto line = qobject_cast<QCPGraph*>(sender());
    if (!line || !selected) return;
    auto item = itemForLine(line);
    if (item)
        emit graphSelected(item->graph);
}

PlotItem* PlotWindow::itemForLine(QCPGraph* line) const
{
    for (auto item : _items)
        if (item->line == line)
            return item;
    return nullptr;
}

PlotItem* PlotWindow::itemForGraph(Graph* graph) const
{
    for (auto item : _items)
        if (item->graph == graph)
            return item;
    return nullptr;
}

Graph* PlotWindow::selectedGraph() const
{
    auto lines = _plot->selectedGraphs();
    if (lines.isEmpty()) return nullptr;
    auto item = itemForLine(lines.first());
    if (!item) return nullptr;
    return item->graph;
}

void PlotWindow::selectGraph(Graph* graph)
{
    auto item = itemForGraph(graph);
    if (!item) return;

    _plot->deselectAll();
    item->line->setSelection(QCPDataSelection(item->line->data()->dataRange()));
    _plot->replot();
}
