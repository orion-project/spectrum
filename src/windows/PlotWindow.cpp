#include "PlotWindow.h"

#include "../core/Graph.h"
#include "../core/GraphMath.h"
#include "../app/PersistentState.h"
#include "../app/AppSettings.h"
#include "../Operations.h"

#include "helpers/OriLayouts.h"
#include "helpers/OriDialogs.h"
#include "tools/OriMessageBus.h"
#include "widgets/OriFlatToolBar.h"
#include "widgets/OriPopupMessage.h"

#include "qcpl_plot.h"
#include "qcpl_colors.h"
#include "qcpl_cursor.h"
#include "qcpl_cursor_panel.h"
#include "qcpl_format.h"
#include "qcpl_io_json.h"

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

PlotWindow::PlotWindow(Operations *operations, QWidget *parent) : QWidget(parent), _operations(operations)
{
    static int plotIndex = 0;

    _plotObj = new PlotObj;
    _plotObj->_icon = nextPlotIcon();

    _plot = new QCPL::Plot;
    _plot->formatAxisTitleAfterFactorSet = true;
    _plot->addTextVarX("{factor}", tr("Axis factor"), [this]{ return QCPL::axisFactorStr(_plot->axisFactorX()); });
    _plot->addTextVarY("{factor}", tr("Axis factor"), [this]{ return QCPL::axisFactorStr(_plot->axisFactorY()); });
    _plot->addTextVarX("{(factor)}", tr("Axis factor (in brackets)"), [this]{
        auto s = QCPL::axisFactorStr(_plot->axisFactorX()); return s.isEmpty() ? QString() : QStringLiteral("(%1)").arg(s); });
    _plot->addTextVarY("{(factor)}", tr("Axis factor (in brackets)"), [this]{
        auto s = QCPL::axisFactorStr(_plot->axisFactorY()); return s.isEmpty() ? QString() : QStringLiteral("(%1)").arg(s); });
    _plot->setPlottingHint(QCP::phFastPolylines, true);
    connect(_plot, &QCPL::Plot::modified, this, &PlotWindow::markModified);

    _cursor = new QCPL::Cursor(_plot);
    _plot->serviceGraphs().append(_cursor);

    auto toolbar = new Ori::Widgets::FlatToolBar;
    toolbar->setIconSize({16, 16});

    _cursorPanel = new QCPL::CursorPanel(_cursor);
    _cursorPanel->placeIn(toolbar);

    createContextMenus();

    Ori::Layouts::LayoutV({toolbar, _plot}).setMargin(0).setSpacing(0).useFor(this);

    setWindowIcon(_plotObj->icon());
    updateTitle(tr("Diagram %1").arg(++plotIndex));
}

PlotWindow::~PlotWindow()
{
    qDeleteAll(_items);
    delete _plotObj;
}

void PlotWindow::createContextMenus()
{
    auto menuX = new QMenu(this);
    auto titleX = new QWidgetAction(this);
    auto labelX = new QLabel(tr("<b>Axis X</b>"));
    labelX->setMargin(6);
    titleX->setDefaultWidget(labelX);
    menuX->addAction(titleX);
    menuX->addAction(QIcon(":/toolbar/title_x"), tr("Text..."), _plot, &QCPL::Plot::axisTextDlgX);
    menuX->addAction(QIcon(":/toolbar/format_x"), tr("Format..."), this, [this]{ formatX(); });
    menuX->addSeparator();
    menuX->addAction(QIcon(":/toolbar/copy_fmt"), tr("Copy Format"), this, [this]{ QCPL::copyAxisFormat(_plot->xAxis); });
    menuX->addAction(QIcon(":/toolbar/paste_fmt"), tr("Paste Format"), this, [this]{ pasteAxisFormat(_plot->xAxis); });
    menuX->addSeparator();
    menuX->addAction(QIcon(":/toolbar/limits_x"), tr("Limits..."), this, [this]{ limitsDlgX(); });
    menuX->addAction(QIcon(":/toolbar/limits_auto_x"), tr("Fit to Graphs"), this, [this]{ autolimitsX(); });
    menuX->addAction(QIcon(":/toolbar/limits_fit_x"), tr("Fit to Selection"), this, [this]{ limitsToSelectionX(); });
    menuX->addSeparator();
    menuX->addAction(tr("Factor..."), _plot, &QCPL::Plot::axisFactorDlgX);

    auto menuY = new QMenu(this);
    auto titleY = new QWidgetAction(this);
    auto labelY = new QLabel(tr("<b>Axis Y</b>"));
    labelY->setMargin(6);
    titleY->setDefaultWidget(labelY);
    menuY->addAction(titleY);
    menuY->addAction(QIcon(":/toolbar/title_y"), tr("Text..."), _plot, &QCPL::Plot::axisTextDlgY);
    menuY->addAction(QIcon(":/toolbar/format_y"), tr("Format..."), this, [this]{ formatY(); });
    menuY->addSeparator();
    menuY->addAction(QIcon(":/toolbar/copy_fmt"), tr("Copy Format"), this, [this]{ QCPL::copyAxisFormat(_plot->yAxis); });
    menuY->addAction(QIcon(":/toolbar/paste_fmt"), tr("Paste Format"), this, [this]{ pasteAxisFormat(_plot->yAxis); });
    menuY->addSeparator();
    menuY->addAction(QIcon(":/toolbar/limits_y"), tr("Limits..."), this, [this]{ limitsDlgY(); });
    menuY->addAction(QIcon(":/toolbar/limits_auto_y"), tr("Fit to Graphs"), this, [this]{ limitsToSelectionY(); });
    menuY->addAction(QIcon(":/toolbar/limits_fit_y"), tr("Fit to Selection"), this, [this]{ limitsToSelectionY(); });
    menuY->addSeparator();
    menuY->addAction(tr("Factor..."), _plot, &QCPL::Plot::axisFactorDlgY);

    auto menuLegend = new QMenu(this);
    menuLegend->addAction(QIcon(":/toolbar/plot_legend"), tr("Format..."), this, [this]{ formatLegend(); });
    menuLegend->addSeparator();
    menuLegend->addAction(QIcon(":/toolbar/copy_fmt"), tr("Copy Format"), this, [this]{ QCPL::copyLegendFormat(_plot->legend); });
    menuLegend->addAction(QIcon(":/toolbar/paste_fmt"), tr("Paste Format"), this, [this]{ pasteLegendFormat(); });

    auto menuTitle = new QMenu(this);
    menuTitle->addAction(QIcon(":/toolbar/plot_title"), tr("Text..."), _plot, &QCPL::Plot::titleTextDlg);
    menuTitle->addAction(QIcon(":/toolbar/plot_title"), tr("Format..."), this, [this]{ editTitle(); });
    menuTitle->addSeparator();
    menuTitle->addAction(QIcon(":/toolbar/copy_fmt"), tr("Copy Format"), this, [this]{ QCPL::copyTitleFormat(_plot->title()); });
    menuTitle->addAction(QIcon(":/toolbar/paste_fmt"), tr("Paste Format"), this, [this]{ pasteTitleFormat(); });

    auto menuGraph = new QMenu(this);
    menuGraph->addAction(QIcon(":/toolbar/graph_title"), tr("Title..."), _operations, &Operations::graphTitle);
    menuGraph->addAction(QIcon(":/toolbar/graph_props"), tr("Format..."), this, [this]{ formatGraph(); });
    menuGraph->addSeparator();
    menuGraph->addAction(QIcon(":/toolbar/copy_img"), tr("Copy Image"), this, [this]{ copyPlotImage(); });

    auto menuPlot = new QMenu(this);
    menuPlot->addAction(QIcon(":/toolbar/copy_img"), tr("Copy Image"), this, [this]{ copyPlotImage(); });
    menuPlot->addSeparator();
    menuPlot->addAction(QIcon(":/toolbar/copy_fmt"), tr("Copy Format"), this, [this]{ copyPlotFormat(); });
    menuPlot->addAction(QIcon(":/toolbar/paste_fmt"), tr("Paste Format"), this, [this]{ pastePlotFormat(); });

    _plot->menuAxisX = menuX;
    _plot->menuAxisY = menuY;
    _plot->menuGraph = menuGraph;
    _plot->menuPlot = menuPlot;
    _plot->menuLegend = menuLegend;
    _plot->menuTitle = menuTitle;
}

void PlotWindow::closeEvent(class QCloseEvent* ce)
{
    QWidget::closeEvent(ce);

    if (graphCount() == 0 or
        Ori::Dlg::yes(tr("Delete diagram <b>%1</b> and all its graphs?").arg(windowTitle())))
        ce->accept();
    else
        ce->ignore();
}

void PlotWindow::updateTitle(const QString& title)
{
    QString oldTitle = _plotObj->_title;
    _plotObj->_title = title;
    setWindowTitle(title);
    if (_plot->title() and _plot->title()->text() == oldTitle)
        _plot->title()->setText(title);
}

void PlotWindow::markModified(const QString &reason)
{
    // TODO
    qDebug() << "Modified" << reason;
}

int PlotWindow::graphCount() const
{
    return _plot->graphsCount();
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
    _plot->updateTitleVisibility();
    _plot->replot();
    markModified("PlotWindow::setTitleVisible");
}

void PlotWindow::editTitle()
{
    _plot->titleFormatDlg();
}

void PlotWindow::formatX()
{
    _plot->axisFormatDlgX();
}

void PlotWindow::formatY()
{
    _plot->axisFormatDlgY();
}

void PlotWindow::formatLegend()
{
    _plot->legendFormatDlg();
}

void PlotWindow::formatGraph()
{
    auto lines = _plot->selectedGraphs();
    if (lines.isEmpty()) {
        PopupMessage::warning(tr("Graphs not selected"));
        return;
    }

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

void PlotWindow::pasteTitleFormat()
{
    auto err = QCPL::pasteTitleFormat(_plot->title());
    if (err.isEmpty())
    {
        _plot->replot();
        markModified("PlotWindow::pasteTitleFormat");
    }
    else Ori::Dlg::info(err);
}

void PlotWindow::pasteLegendFormat()
{
    auto err = QCPL::pasteLegendFormat(_plot->legend);
    if (err.isEmpty())
    {
        _plot->replot();
        markModified("PlotWindow::pasteLegendFormat");
    }
    else Ori::Dlg::info(err);
}

void PlotWindow::pasteAxisFormat(QCPAxis *axis)
{
    auto err = QCPL::pasteAxisFormat(axis);
    if (err.isEmpty())
    {
        _plot->replot();
        markModified("PlotWindow::pasteAxisFormat");
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

void PlotWindow::rename()
{
    QString newTitle = Ori::Dlg::inputText(tr("Diagram title:"), _plotObj->title());
    if (newTitle.isEmpty()) return;
    updateTitle(newTitle);
    markModified("PlotWindow::rename");
    _plot->replot(); // Update title in QCPL::Plot
    Ori::MessageBus::instance().send(MSG_PLOT_RENAMED, {{"plotId", _plotObj->id()}});
}
