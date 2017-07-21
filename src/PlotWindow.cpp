#include "PlotWindow.h"
#include "qcpl_plot.h"
#include "qcpl_colors.h"
#include "core/Graph.h"
#include "helpers/OriLayouts.h"
#include "qcpl_colors.h"

PlotItem::~PlotItem()
{
    delete graph;
}

QIcon nextPlotIcon()
{
    static int nextColorIndex = 0;
    if (nextColorIndex == QCPL::defaultColorSet().size())
        nextColorIndex = 0;
    QColor c = QCPL::defaultColorSet().at(nextColorIndex++);

    int H, S, L;
    c.getHsl(&H, &S, &L);
    QColor backColor = QColor::fromHsl(H, S*0.8, L*1.2);
    QColor borderColor = QColor::fromHsl(H, S*0.5, L);

    QPixmap px(16, 16);
    px.fill(Qt::transparent);

    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing, true);

    QPen borderPen(borderColor);
    borderPen.setWidthF(1.5);
    p.setPen(borderPen);

    p.setBrush(backColor);
    p.drawEllipse(px.rect().adjusted(1, 1, -1, -1));

    // TODO draw gradient gloss

    return QIcon(px);
}

//------------------------------------------------------------------------------

PlotWindow::PlotWindow(QWidget *parent) : QWidget(parent)
{
    setWindowIcon(nextPlotIcon());

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
    item->line->setName(g->title());
    item->line->setSelectable(QCP::stWhole);
    item->line->setData(g->x(), g->y());
    item->line->setPen(nextGraphColor());
    connect(item->line, SIGNAL(selectionChanged(bool)), this, SLOT(graphLineSelected(bool)));
    _items.append(item);
    _plot->replot();
}

void PlotWindow::autolimits()
{
    _plot->autolimits();
}

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

bool PlotWindow::isLegendVisible() const
{
    return _plot->legend->visible();
}

void PlotWindow::setLegendVisible(bool on)
{
    _plot->legend->setVisible(on);
    _plot->replot();
}

QColor PlotWindow::nextGraphColor()
{
    if (_nextColorIndex == QCPL::defaultColorSet().size())
        _nextColorIndex = 0;
    return QCPL::defaultColorSet().at(_nextColorIndex++);
}
