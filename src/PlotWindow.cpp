#include "PlotWindow.h"
#include "qcpl_plot.h"
#include "qcpl_colors.h"
#include "core/Graph.h"
#include "helpers/OriLayouts.h"
#include "helpers/OriDialogs.h"
#include "qcpl_colors.h"

PlotItem::~PlotItem()
{
    delete graph;
}

static QIcon makeGraphIcon(QColor color)
{
    int H, S, L;
    color.getHsl(&H, &S, &L);
    QColor backColor = QColor::fromHsl(H, int(float(S)*0.8f), int(float(L)*1.2f));
    QColor borderColor = QColor::fromHsl(H, int(float(S)*0.5f), L);

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

static QIcon nextPlotIcon()
{
    static int nextColorIndex = 0;
    if (nextColorIndex == QCPL::defaultColorSet().size())
        nextColorIndex = 0;

    QColor c = QCPL::defaultColorSet().at(nextColorIndex++);
    return makeGraphIcon(c);
}

//------------------------------------------------------------------------------

PlotWindow::PlotWindow(QWidget *parent) : QWidget(parent)
{
    static int plotIndex = 0;

    _plotObj = new PlotObj;
    _plotObj->_icon = nextPlotIcon();
    _plotObj->_title = tr("Plot %1").arg(++plotIndex);

    setWindowIcon(_plotObj->icon());
    setWindowTitle(_plotObj->title());

    _plot = new QCPL::Plot;
    _plot->setPlottingHint(QCP::phFastPolylines, true);
    if (_plot->title())
        _plot->title()->setText(_plotObj->title());
    connect(_plot, &QCPL::Plot::editTitleRequest, this, &PlotWindow::editTitle);

    Ori::Layouts::LayoutV({_plot}).setMargin(0).setSpacing(0).useFor(this);
}

PlotWindow::~PlotWindow()
{
    qDeleteAll(_items);
    delete _plotObj;
}

void PlotWindow::addGraph(Graph* g)
{
    auto item = new PlotItem;
    item->graph = g;

    item->line = _plot->makeNewGraph(g->title(), g->x(), g->y());
    connect(item->line, SIGNAL(selectionChanged(bool)), this, SLOT(graphLineSelected(bool)));

    g->setColor(item->line->pen().color());
    g->setIcon(makeGraphIcon(g->color()));

    _items.append(item);
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

bool PlotWindow::updateGraph(Graph* graph)
{
    auto item = itemForGraph(graph);
    if (!item) return false;

    _plot->updateGraph(item->line, graph->x(), graph->y());
    return true;
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

bool PlotWindow::isTitleVisible() const
{
    return _plot->title() && _plot->title()->visible();
}

void PlotWindow::setTitleVisible(bool on)
{
    _plot->setTitleVisible(on);
    if (on)
        _plot->title()->setText(_plotObj->title());
    _plot->replot();
}

void PlotWindow::editTitle()
{
    bool ok;
    QString newTitle = Ori::Dlg::inputText(tr("Plot title"), _plot->title()->text(), &ok);
    if (!ok) return;
    _plot->title()->setText(newTitle);
    _plot->replot();
    // TODO should we update _plotObj::title and plot window title as well? or should they be different titles?
}
