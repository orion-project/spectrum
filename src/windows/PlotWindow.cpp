#include "PlotWindow.h"

#include "../core/Graph.h"
#include "../core/GraphMath.h"
#include "../app/PersistentState.h"
#include "../app/AppSettings.h"

#include "helpers/OriLayouts.h"
#include "helpers/OriDialogs.h"
#include "widgets/OriFlatToolBar.h"
#include "widgets/OriPopupMessage.h"

#include "qcpl_plot.h"
#include "qcpl_colors.h"
#include "qcpl_cursor.h"
#include "qcpl_cursor_panel.h"
#include "qcpl_format.h"
#include "qcpl_io_json.h"

//using PopupMessage = Ori::Gui::PopupMessage;
using Ori::Gui::PopupMessage;

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

PlotWindow::PlotWindow(QWidget *parent) : QWidget(parent)
{
    static int plotIndex = 0;

    _plotObj = new PlotObj;
    _plotObj->_icon = nextPlotIcon();
    _plotObj->_title = tr("Diagram %1").arg(++plotIndex);

    setWindowIcon(_plotObj->icon());
    setWindowTitle(_plotObj->title());

    _plot = new QCPL::Plot;
    _plot->setPlottingHint(QCP::phFastPolylines, true);
    if (_plot->title())
        _plot->title()->setText(_plotObj->title());
    //connect(_plot, &QCPL::Plot::editTitleRequest, this, &PlotWindow::editTitle);

    _cursor = new QCPL::Cursor(_plot);
    _plot->serviceGraphs().append(_cursor);

    auto toolbar = new Ori::Widgets::FlatToolBar;
    toolbar->setIconSize({16, 16});

    _cursorPanel = new QCPL::CursorPanel(_cursor);
    _cursorPanel->placeIn(toolbar);

    Ori::Layouts::LayoutV({toolbar, _plot}).setMargin(0).setSpacing(0).useFor(this);
}

PlotWindow::~PlotWindow()
{
    qDeleteAll(_items);
    delete _plotObj;
}

void PlotWindow::closeEvent(class QCloseEvent* ce)
{
    QWidget::closeEvent(ce);

    if (Ori::Dlg::yes(tr("Delete diagram <b>%1</b> and all its graphs?").arg(windowTitle())))
        ce->accept();
    else
        ce->ignore();
}

void PlotWindow::markModified(const char *reason)
{
    // TODO
    qDebug() << "Modified" << reason;
}

void PlotWindow::addGraph(Graph* g)
{
    auto item = new PlotItem;
    item->graph = g;

    item->line = _plot->makeNewGraph(g->title(), {g->data().xs, g->data().ys});
    connect(item->line, SIGNAL(selectionChanged(bool)), this, SLOT(graphLineSelected(bool)));

    g->setColor(item->line->pen().color());
    g->setIcon(makeGraphIcon(g->color()));

    _items.append(item);
}

void PlotWindow::limitsDlg()
{
    if (_plot->limitsDlgXY())

        // TODO if limits changed
        markModified("PlotWindow::limitsDlg");
}

void PlotWindow::limitsDlgX()
{
    if (_plot->limitsDlgX())
        // TODO if limits changed
        markModified("PlotWindow::limitsDlgX");
}

void PlotWindow::limitsDlgY()
{
    if (_plot->limitsDlgY())
        // TODO if limits changed
        markModified("PlotWindow::limitsDlgY");
}

void PlotWindow::autolimits()
{
    _plot->autolimits();
    // TODO if limits changed
    markModified("PlotWindow::autolimits");
}

void PlotWindow::autolimitsX()
{
    _plot->autolimitsX();
    // TODO if limits changed
    markModified("PlotWindow::autolimitsX");
}

void PlotWindow::autolimitsY()
{
    _plot->autolimitsY();
    // TODO if limits changed
    markModified("PlotWindow::autolimitsY");
}

void PlotWindow::limitsToSelection()
{
    // TODO: process multiselection
    auto g = selectedGraph();
    if (!g) return;
    auto minMax = GraphMath::minMax(g->data());
    _plot->setLimitsX(minMax.minX, minMax.maxX, false);
    _plot->setLimitsY(minMax.minY.y, minMax.maxY.y);
    // TODO if limits changed
    markModified("PlotWindow::limitsToSelection");
}

void PlotWindow::limitsToSelectionX()
{
    // TODO: process multiselection
    auto g = selectedGraph();
    if (!g) return;
    // TODO: process only visible part
    auto minMax = GraphMath::minMax(g->data());
    _plot->setLimitsX(minMax.minX, minMax.maxX);
    // TODO if limits changed
    markModified("PlotWindow::limitsToSelectionX");
}

void PlotWindow::limitsToSelectionY()
{
    // TODO: process multiselection
    auto g = selectedGraph();
    if (!g) return;
    // TODO: process only visible part
    auto minMax = GraphMath::minMax(g->data());
    _plot->setLimitsY(minMax.minY.y, minMax.maxY.y);
    // TODO if limits changed
    markModified("PlotWindow::limitsToSelectionY");
}

void PlotWindow::zoomIn()
{
    _plot->zoomIn();
}

void PlotWindow::zoomOut()
{
    _plot->zoomOut();
}

void PlotWindow::zoomInX()
{
    _plot->zoomInX();
}

void PlotWindow::zoomOutX()
{
    _plot->zoomOutX();
}

void PlotWindow::zoomInY()
{
    _plot->zoomInY();
}

void PlotWindow::zoomOutY()
{
    _plot->zoomOutY();
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

    item->line->setName(graph->title());
    _plot->updateGraph(item->line, {graph->data().xs, graph->data().ys});
    return true;
}

bool PlotWindow::isLegendVisible() const
{
    return _plot->legend->visible();
}

void PlotWindow::setLegendVisible(bool on)
{
    if (_plot->legend->visible() == on)
        return;
    _plot->legend->setVisible(on);
    _plot->replot();
    markModified("PlotWindow::setLegendVisible");
}

bool PlotWindow::isTitleVisible() const
{
    return _plot->title() && _plot->title()->visible();
}

void PlotWindow::setTitleVisible(bool on)
{
    if (_plot->title()->visible() == on)
        return;
    _plot->title()->setVisible(on);
    if (on && _plot->title()->text().isEmpty())
        _plot->title()->setText(_plotObj->title());
    _plot->replot();
    markModified("PlotWindow::setTitleVisible");
}

void PlotWindow::editTitle()
{
    if (_plot->titleFormatDlg())
        markModified("PlotWindow::editTitle");
}

void PlotWindow::formatX()
{
    if (_plot->axisFormatDlgX())
        markModified("PlotWindow::formatX");
}

void PlotWindow::formatY()
{
    if (_plot->axisFormatDlgY())
        markModified("PlotWindow::formatY");
}

void PlotWindow::formatLegend()
{
    if (_plot->legendFormatDlg())
        markModified("PlotWindow::formatLegend");
}

void PlotWindow::formatGraph()
{
    auto lines = _plot->selectedGraphs();
    if (lines.isEmpty()) return;

    QCPGraph *line = lines.first();

    QCPL::GraphFormatDlgProps props;
    props.title = tr("Format %1").arg(line->name());
    props.plot = _plot;
    if (QCPL::graphFormatDlg(line, props))
    {
        _plot->replot();
        markModified("PlotWindow::formatGraph");
    }
}

void PlotWindow::copyPlotFormat()
{
    QCPL::copyPlotFormat(_plot);
}

void PlotWindow::pastePlotFormat()
{
    auto err = QCPL::pastePlotFormat(_plot);
    if (err.isEmpty())
    {
        _plot->updateTitleVisibility();
        _plot->replot();
        markModified("PlotWindow::pastePlotFormat");
    }
    else Ori::Dlg::info(err);
}

void PlotWindow::savePlotFormat()
{
    QString recentPath = RecentData::getDir("plot_format_path");
    auto fileName = QFileDialog::getSaveFileName(
        this, tr("Save Plot Format"), recentPath, tr("JSON files (*.json)\nAll files (*.*)"));
    if (fileName.isEmpty())
        return;
    RecentData::setDir("plot_format_path", fileName);
    QString err = QCPL::saveFormatToFile(fileName, _plot);
    if (!err.isEmpty())
        Ori::Dlg::error(err);
}

void PlotWindow::loadPlotFormat()
{
    QString recentPath = RecentData::getDir("plot_format_path");
    auto fileName = QFileDialog::getOpenFileName(
        this, tr("Load Plot Format"), recentPath, tr("JSON files (*.json)\nAll files (*.*)"));
    if (fileName.isEmpty())
        return;
    RecentData::setDir("plot_format_path", fileName);
    QCPL::JsonReport report;
    auto err = QCPL::loadFormatFromFile(fileName, _plot, &report);
    markModified("PlotWindow::loadPlotFormat");
    if (!err.isEmpty())
    {
        Ori::Dlg::error(err);
        return;
    }
    _plot->replot();
}

void PlotWindow::copyPlotImage()
{
    bool oldVisible = _cursor->visible();
    if (AppSettings::instance().exportHideCursor)
        _cursor->setVisible(false);

    QImage image(_plot->width(), _plot->height(), QImage::Format_RGB32);
    QCPPainter painter(&image);
    _plot->toPainter(&painter);
    qApp->clipboard()->setImage(image);

    if (oldVisible != _cursor->visible())
        _cursor->setVisible(oldVisible);

    (new PopupMessage(PopupMessage::AFFIRM,
        tr("Image has been copied to Clipboard"), -1, Qt::AlignRight|Qt::AlignBottom, this))->show();
}
