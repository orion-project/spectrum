#include "PlotWindow.h"

#include "Operations.h"
#include "core/Project.h"
#include "app/PersistentState.h"

#include "helpers/OriLayouts.h"
#include "helpers/OriDialogs.h"
#include "tools/OriMessageBus.h"
#include "tools/OriMruList.h"
#include "widgets/OriPopupMessage.h"

#include "qcpl_axis.h"
//#include "qcpl_cursor.h"
//#include "qcpl_cursor_panel.h"
#include "qcpl_export.h"
#include "qcpl_format.h"
#include "qcpl_io_json.h"
#include "qcpl_plot.h"

using Ori::Gui::PopupMessage;
using Ori::MessageBus;

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

PlotWindow::PlotWindow(Operations *operations, Diagram *diagram, QWidget *parent)
    : QWidget(parent), Ori::IMessageBusListener(), _diagram(diagram), _operations(operations)
{
    _plot = new QCPL::Plot({.replaceDefaultAxes=true});
    _plot->formatAxisTitleAfterFactorSet = true;
    _plot->highlightAxesOfSelectedGraphs = AppSettings::instance().highlightAxesOfSelectedGraphs;;
    foreach (auto axis, _plot->defaultAxes())
        addAxisVars(axis);
    _plot->setPlottingHint(QCP::phFastPolylines, true);
    _plot->setInteraction(QCP::iMultiSelect, true);
    connect(_plot, &QCPL::Plot::modified, _diagram, &Diagram::markModified);

    //_cursor = new QCPL::Cursor(_plot);
    //_plot->serviceGraphs().append(_cursor);

    //auto toolbar = new Ori::Widgets::FlatToolBar;
    //toolbar->setIconSize({16, 16});

    //_cursorPanel = new QCPL::CursorPanel(_cursor);
    //_cursorPanel->placeIn(toolbar);

    createContextMenus();

    Ori::Layouts::LayoutV({
        //toolbar,
        _plot,
    }).setMargin(0).setSpacing(0).useFor(this);

    setWindowIcon(_diagram->icon());
    handleDiagramRenamed();
}

PlotWindow::~PlotWindow()
{
    qDeleteAll(_items);
}

void PlotWindow::createContextMenus()
{
    auto menuAxis = new QMenu(this);
    auto titleAxis = new QWidgetAction(this);
    auto labelAxis = new QLabel;
    labelAxis->setMargin(6);
    labelAxis->setFrameShape(QFrame::StyledPanel);
    titleAxis->setDefaultWidget(labelAxis);
    menuAxis->addAction(titleAxis);
    connect(menuAxis, &QMenu::aboutToShow, this, [this, labelAxis]{
        labelAxis->setText("<b>" + _plot->axisIdent(_plot->axisUnderMenu) + "</b>");
    });
    menuAxis->addAction(tr("Text..."), this, [this]{ _plot->axisTextDlg(_plot->axisUnderMenu); });
    menuAxis->addAction(QIcon(":/toolbar/format_axis"), tr("Format..."), this, [this]{ _plot->axisFormatDlg(_plot->axisUnderMenu); });
    menuAxis->addSeparator();
    menuAxis->addAction(QIcon(":/toolbar/copy_fmt"), tr("Copy Format"), this, [this]{ QCPL::copyAxisFormat(_plot->axisUnderMenu); });
    menuAxis->addAction(QIcon(":/toolbar/paste_fmt"), tr("Paste Format"), this, [this]{ pasteAxisFormat(_plot->axisUnderMenu); });
    menuAxis->addSeparator();
    menuAxis->addAction(QIcon(":/toolbar/limits"), tr("Limits..."), this, [this]{ _plot->limitsDlg(_plot->axisUnderMenu); });
    menuAxis->addAction(QIcon(":/toolbar/limits_auto_lite"), tr("Fit to Graphs"), this, [this]{ _plot->autolimits(_plot->axisUnderMenu, true); });
    menuAxis->addSeparator();
    menuAxis->addAction(QIcon(":/toolbar/factor_axis"), tr("Factor..."), this, [this]{
        if (_plot->axisFactorDlg(_plot->axisUnderMenu))
            _diagram->markModified("PlotWindow::axisFactorChanged");
    });

    auto menuLegend = new QMenu(this);
    menuLegend->addAction(QIcon(":/toolbar/plot_legend"), tr("Format..."), this, &PlotWindow::formatLegend);
    menuLegend->addSeparator();
    menuLegend->addAction(QIcon(":/toolbar/copy_fmt"), tr("Copy Format"), this, [this]{ QCPL::copyLegendFormat(_plot->legend); });
    menuLegend->addAction(QIcon(":/toolbar/paste_fmt"), tr("Paste Format"), this, &PlotWindow::pasteLegendFormat);

    auto menuTitle = new QMenu(this);
    menuTitle->addAction(QIcon(":/toolbar/plot_title"), tr("Text..."), _plot, &QCPL::Plot::titleTextDlg);
    menuTitle->addAction(QIcon(":/toolbar/plot_title"), tr("Format..."), this, &PlotWindow::formatTitle);
    menuTitle->addSeparator();
    menuTitle->addAction(QIcon(":/toolbar/copy_fmt"), tr("Copy Format"), this, [this]{ QCPL::copyTitleFormat(_plot->title()); });
    menuTitle->addAction(QIcon(":/toolbar/paste_fmt"), tr("Paste Format"), this, &PlotWindow::pasteTitleFormat);

    auto menuGraph = new QMenu(this);
    menuGraph->addAction(QIcon(":/toolbar/graph_title"), tr("Title..."), this, &PlotWindow::renameGraph);
    menuGraph->addAction(QIcon(":/toolbar/graph_format"), tr("Format..."), this, &PlotWindow::formatGraph);
    menuGraph->addAction(tr("Change Axes..."), this, &PlotWindow::changeGraphAxes);
    menuGraph->addSeparator();
    menuGraph->addAction(QIcon(":/toolbar/copy_fmt"), tr("Copy Format"), this, &PlotWindow::copyGraphFormat);
    menuGraph->addAction(QIcon(":/toolbar/paste_fmt"), tr("Paste Format"), this, &PlotWindow::pasteGraphFormat);
    menuGraph->addSeparator();
    menuGraph->addAction(QIcon(":/toolbar/graph_delete"), tr("Delete"), this, &PlotWindow::deleteGraph);

    auto menuPlot = new QMenu(this);
    menuPlot->addAction(QIcon(":/toolbar/copy_img"), tr("Copy Image"), this, &PlotWindow::copyPlotImage);
    menuPlot->addSeparator();
    menuPlot->addAction(QIcon(":/toolbar/copy_fmt"), tr("Copy Format"), this, &PlotWindow::copyPlotFormat);
    menuPlot->addAction(QIcon(":/toolbar/paste_fmt"), tr("Paste Format"), this, &PlotWindow::pastePlotFormat);

    _plot->menuAxis = menuAxis;
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
    {
        ce->accept();
        _diagram->project()->deleteDiagram(_diagram->id());
    }
    else
        ce->ignore();
}

void PlotWindow::messageBusEvent(int event, const QMap<QString, QVariant>& params)
{
    switch (event) {
    case BusEvent::DiagramRenamed:
        if (params.value("id") == _diagram->id())
            handleDiagramRenamed();
        break;
    case BusEvent::GraphAdded:
        handleGraphAdded(params.value("id").toString());
        break;
    case BusEvent::GraphUpdated:
        handleGraphUpdated(params.value("id").toString());
        break;
    case BusEvent::GraphDeleting:
        handleGraphDeleting(params.value("id").toString());
        break;
    }
}

void PlotWindow::addAxisVars(QCPAxis* axis)
{
    _plot->addTextVar(axis, "{factor}", tr("Axis factor"), [this, axis]{ return QCPL::axisFactorStr(_plot->axisFactor(axis)); });
    _plot->addTextVar(axis, "{(factor)}", tr("Axis factor (in brackets)"), [this, axis]{
        auto s = QCPL::axisFactorStr(_plot->axisFactor(axis)); return s.isEmpty() ? QString() : QStringLiteral("(%1)").arg(s); });
}

void PlotWindow::handleDiagramRenamed()
{
    setWindowTitle(_diagram->title());
    if (_plot->title()) {
        _plot->title()->setText(_diagram->title());
        _plot->replot();
    }
}

int PlotWindow::graphCount() const
{
    return _plot->graphsCount();
}

void PlotWindow::handleGraphAdded(const QString &id)
{
    auto g = _diagram->graph(id);
    if (!g) return;

    auto item = new PlotItem;
    item->graph = g;

    item->line = _plot->makeNewGraph(g->title(), {g->data().xs, g->data().ys}, false);
    connect(item->line, SIGNAL(selectionChanged(bool)), this, SLOT(graphLineSelected(bool)));

    g->setColor(item->line->pen().color());
    g->setIcon(makeGraphIcon(g->color()));

    if (AppSettings::instance().autolimitAfterGraphGreated)
    {
        _plot->autolimits(item->line->keyAxis(), false);
        _plot->autolimits(item->line->valueAxis(), false);
    }
    if (AppSettings::instance().selectNewGraph)
        selectGraphLine(item->line, false);

    _plot->replot();
    _items.append(item);

    if (AppSettings::instance().selectNewGraph)
        emit graphSelected(g);
}

void PlotWindow::handleGraphUpdated(const QString &id)
{
    auto graph = _diagram->graph(id);
    if (!graph) return;

    auto item = itemForGraph(graph);
    if (!item) return;

    item->line->setName(graph->title());
    _plot->updateGraph(item->line, {graph->data().xs, graph->data().ys});
}

void PlotWindow::handleGraphRenamed(const QString &id)
{
    auto graph = _diagram->graph(id);
    if (!graph) return;

    auto item = itemForGraph(graph);
    if (!item) return;

    item->line->setName(graph->title());
    _plot->replot();
}

void PlotWindow::handleGraphDeleting(const QString &id)
{
    auto graph = _diagram->graph(id);
    if (!graph) return;

    auto item = itemForGraph(graph);
    if (!item) return;

    _plot->removeGraph(item->line);
    _items.removeAll(item);
    delete item;

    _plot->replot();
}

void PlotWindow::deleteGraph()
{
    auto graphs = selectedGraphs();
    if (graphs.empty()) return;

    QStringList msg;
    msg << tr("These graphs will be deleted:") << "<br><br>";
    for (auto g : std::as_const(graphs))
        msg << "<b>" << g->title() << "</b><br>";
    msg << "<br>" << tr("Confirm?");

    if (Ori::Dlg::yes(msg.join("")))
        _diagram->deleteGraphs(graphs);
}

void PlotWindow::limitsDlg()
{
    if (_plot->limitsDlgXY())
        // TODO if limits changed
        _diagram->markModified("PlotWindow::limitsDlg");
}

void PlotWindow::limitsDlgX()
{
    if (_plot->limitsDlgX())
        // TODO if limits changed
        _diagram->markModified("PlotWindow::limitsDlgX");
}

void PlotWindow::limitsDlgY()
{
    if (_plot->limitsDlgY())
        // TODO if limits changed
        _diagram->markModified("PlotWindow::limitsDlgY");
}

void PlotWindow::autolimits()
{
    _plot->autolimits();
    // TODO if limits changed
    _diagram->markModified("PlotWindow::autolimits");
}

void PlotWindow::autolimitsX()
{
    _plot->autolimitsX();
    // TODO if limits changed
    _diagram->markModified("PlotWindow::autolimitsX");
}

void PlotWindow::autolimitsY()
{
    _plot->autolimitsY();
    // TODO if limits changed
    _diagram->markModified("PlotWindow::autolimitsY");
}
/*
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
*/
void PlotWindow::zoomIn() { _plot->zoomIn(); }
void PlotWindow::zoomOut() { _plot->zoomOut(); }
void PlotWindow::zoomInX() { _plot->zoomInX(); }
void PlotWindow::zoomOutX() { _plot->zoomOutX(); }
void PlotWindow::zoomInY() { _plot->zoomInY(); }
void PlotWindow::zoomOutY() { _plot->zoomOutY(); }

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

Graph* PlotWindow::selectedGraph(bool warn) const
{
    if (_items.size() == 1)
        return _items.first()->graph;
    auto lines = _plot->selectedGraphs();
    if (lines.isEmpty()) {
        if (warn)
            PopupMessage::warning(tr("Should select a graph"));
        return nullptr;
    }
    auto item = itemForLine(lines.first());
    return item ? item->graph : nullptr;
}

QVector<Graph*> PlotWindow::selectedGraphs(bool warn) const
{
    if (_items.size() == 1)
        return { _items.first()->graph };
    QVector<Graph*> res;
    foreach (auto line, _plot->selectedGraphs())
        if (auto item = itemForLine(line); item)
            res << item->graph;
    if (res.isEmpty() and warn)
        PopupMessage::warning(qApp->tr("Should select a graph"));
    return res;
}

QCPGraph* PlotWindow::selectedGraphLine(bool warn) const
{
    if (_items.size() == 1)
        return _items.first()->line;
    auto lines = _plot->selectedGraphs();
    if (lines.isEmpty()) {
        if (warn)
            PopupMessage::warning(tr("Should select a graph"));
        return nullptr;
    }
    return lines.first();
}

void PlotWindow::selectGraph(Graph* graph)
{
    auto item = itemForGraph(graph);
    if (item) selectGraphLine(item->line);
}

void PlotWindow::selectGraphLine(QCPGraph* line, bool replot)
{
    _plot->deselectAll();
    line->setSelection(QCPDataSelection(line->data()->dataRange()));
    if (replot) _plot->replot();
}

bool PlotWindow::isLegendVisible() const
{
    return _plot->legend->visible();
}

void PlotWindow::toggleLegend()
{
    _plot->legend->setVisible(!_plot->legend->visible());
    _plot->replot();
    _diagram->markModified("PlotWindow::setLegendVisible");
}

bool PlotWindow::isTitleVisible() const
{
    return _plot->title() && _plot->title()->visible();
}

void PlotWindow::toggleTitle()
{
    _plot->title()->setVisible(!_plot->title()->visible());
    if (_plot->title()->visible() && _plot->title()->text().isEmpty())
        _plot->title()->setText(_diagram->title());
    _plot->updateTitleVisibility();
    _plot->replot();
    _diagram->markModified("PlotWindow::setTitleVisible");
}

void PlotWindow::axisFactorDlgX()
{
    if (_plot->axisFactorDlgX())
        _diagram->markModified("PlotWindow::axisFactorChanged");
}

void PlotWindow::axisFactorDlgY()
{
    if (_plot->axisFactorDlgY())
        _diagram->markModified("PlotWindow::axisFactorChanged");
}

void PlotWindow::axisFactorDlg()
{
    auto axis = QCPL::chooseAxis(_plot);
    if (axis && _plot->axisFactorDlg(axis))
        _diagram->markModified("PlotWindow::axisFactorChanged");
}

void PlotWindow::formatX()
{
    _plot->axisFormatDlgX();
}

void PlotWindow::formatY()
{
    _plot->axisFormatDlgY();
}

void PlotWindow::formatX2()
{
    _plot->axisFormatDlg(_plot->xAxis2);
}

void PlotWindow::formatY2()
{
    _plot->axisFormatDlg(_plot->yAxis2);
}

void PlotWindow::formatAxis()
{
    auto axis = QCPL::chooseAxis(_plot);
    if (axis) _plot->axisFormatDlg(axis);
}

void PlotWindow::formatTitle()
{
    _plot->titleFormatDlg();
}

void PlotWindow::formatLegend()
{
    _plot->legendFormatDlg();
}

void PlotWindow::formatGraph()
{
    auto line = selectedGraphLine();
    if (!line) return;

    QCPL::GraphFormatDlgProps props;
    props.title = tr("Format %1").arg(line->name());
    if (QCPL::graphFormatDlg(line, props))
        _diagram->markModified("PlotWindow::formatGraph");
}

void PlotWindow::addAxisBottom()
{
    auto axis = _plot->addAxis(QCPAxis::atBottom);
    addAxisVars(axis);
    _plot->replot();
}

void PlotWindow::addAxisLeft()
{
    auto axis = _plot->addAxis(QCPAxis::atLeft);
    addAxisVars(axis);
    _plot->replot();
}

void PlotWindow::addAxisTop()
{
    auto axis = _plot->addAxis(QCPAxis::atTop);
    addAxisVars(axis);
    _plot->replot();
}

void PlotWindow::addAxisRight()
{
    auto axis = _plot->addAxis(QCPAxis::atRight);
    addAxisVars(axis);
    _plot->replot();
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
        _diagram->markModified("PlotWindow::pastePlotFormat");
    }
    else PopupMessage::warning(err);
}

void PlotWindow::pasteTitleFormat()
{
    auto err = QCPL::pasteTitleFormat(_plot->title());
    if (err.isEmpty())
    {
        _plot->replot();
        _diagram->markModified("PlotWindow::pasteTitleFormat");
    }
    else PopupMessage::warning(err);
}

void PlotWindow::pasteLegendFormat()
{
    auto err = QCPL::pasteLegendFormat(_plot->legend);
    if (err.isEmpty())
    {
        _plot->replot();
        _diagram->markModified("PlotWindow::pasteLegendFormat");
    }
    else PopupMessage::warning(err);
}

void PlotWindow::pasteAxisFormat(QCPAxis *axis)
{
    auto err = QCPL::pasteAxisFormat(axis);
    if (err.isEmpty())
    {
        _plot->replot();
        _diagram->markModified("PlotWindow::pasteAxisFormat");
    }
    else PopupMessage::warning(err);
}

void PlotWindow::copyGraphFormat()
{
    auto line = selectedGraphLine();
    if (line) QCPL::copyGraphFormat(line);
}

void PlotWindow::pasteGraphFormat()
{
    auto line = selectedGraphLine();
    if (!line) return;

    auto err = QCPL::pasteGraphFormat(line);
    if (err.isEmpty())
    {
        _plot->replot();
        _diagram->markModified("PlotWindow::pasteGraphFormat");
    }
    else PopupMessage::warning(err);
}

void PlotWindow::savePlotFormatDlg()
{
    QString recentPath = RecentData::getDir("plot_format_path");
    auto fileName = QFileDialog::getSaveFileName(
        this, tr("Save Plot Format"), recentPath, tr("JSON files (*.json)\nAll files (*.*)"));
    if (fileName.isEmpty())
        return;
    RecentData::setDir("plot_format_path", fileName);
    QString err = QCPL::saveFormatToFile(fileName, _plot, {.onlyPrimaryAxes=false});
    if (!err.isEmpty())
        Ori::Dlg::error(err);
}

void PlotWindow::loadPlotFormatDlg()
{
    QString recentPath = RecentData::getDir("plot_format_path");
    auto fileName = QFileDialog::getOpenFileName(
        this, tr("Load Plot Format"), recentPath, tr("JSON files (*.json)\nAll files (*.*)"));
    if (fileName.isEmpty())
        return;
    RecentData::setDir("plot_format_path", fileName);
    loadPlotFormat(fileName);
}

void PlotWindow::loadPlotFormat(const QString& fileName)
{
    QCPL::JsonReport report;
    auto err = QCPL::loadFormatFromFile(fileName, _plot, &report, {.autoCreateAxes=true});
    _diagram->markModified("PlotWindow::loadPlotFormat");
    if (!err.isEmpty())
    {
        Ori::Dlg::error(err);
        return;
    }
    _operations->mruPlotFormats()->append(fileName);
    _plot->replot();
}

void PlotWindow::copyPlotImage()
{
    //bool oldVisible = _cursor->visible();
    //if (AppSettings::instance().exportHideCursor)
    //    _cursor->setVisible(false);

    QImage image(_plot->width(), _plot->height(), QImage::Format_RGB32);
    QCPPainter painter(&image);
    _plot->toPainter(&painter);
    qApp->clipboard()->setImage(image);

    //if (oldVisible != _cursor->visible())
    //    _cursor->setVisible(oldVisible);

    PopupMessage::affirm(tr("Image has been copied to Clipboard"), Qt::AlignRight|Qt::AlignBottom);
}

void PlotWindow::renamePlot()
{
    QString newTitle = Ori::Dlg::inputText(tr("Diagram title:"), _diagram->title());
    if (newTitle.isEmpty() || newTitle == _diagram->title()) return;
    _diagram->setTitle(newTitle);
    MessageBus::send((int)BusEvent::DiagramRenamed, {{"id", _diagram->id()}});
    _diagram->markModified("PlotWindow::renamePlot");
}

void PlotWindow::renameGraph()
{
    auto graph = selectedGraph();
    if (!graph) return;
    QString newTitle = Ori::Dlg::inputText(tr("Graph title:"), graph->title());
    if (newTitle.isEmpty() || newTitle == graph->title()) return;
    graph->setTitle(newTitle);
    MessageBus::send((int)BusEvent::GraphRenamed, {{"id", graph->id()}});
    _diagram->markModified("PlotWindow::renameGraph");
}

QString PlotWindow::displayFactorX() const
{
    QString s = QCPL::axisFactorStr(_plot->axisFactorX());
    // msvc doesn't undestand "strange" symbols in QStringLiteral
    return s.isEmpty() ? QString("X: ×1") : QStringLiteral("X: ") + s;
}

QString PlotWindow::displayFactorY() const
{
    QString s = QCPL::axisFactorStr(_plot->axisFactorY());
    // msvc doesn't undestand "strange" symbols in QStringLiteral
    return s.isEmpty() ? QString("Y: ×1") : QStringLiteral("Y: ") + s;
}

void PlotWindow::exportPlotImg()
{
    QCPL::ExportToImageProps props;
    props.fromJson(PersistentState::load("export_img"));
    if (QCPL::exportImageDlg(_plot, props)) {
        PersistentState::save("export_img", props.toJson());
        PopupMessage::affirm(tr("Image has been saved\n%1").arg(props.fileName), Qt::AlignRight|Qt::AlignBottom);
    }
}

void PlotWindow::exportPlotPrj()
{
}

void PlotWindow::changeGraphAxes()
{
    auto graph = selectedGraphLine();
    if (!graph) return;

    bool changed = false;
    auto res = QCPL::chooseAxes(_plot, {graph->keyAxis(), graph->valueAxis()});
    if (res.x and res.x != graph->keyAxis()) {
        changed = true;
        res.x->setVisible(true);
        graph->setKeyAxis(res.x);
        if (AppSettings::instance().autolitmAfterAxesChanged)
            _plot->autolimits(res.x, false);
    }
    if (res.y and res.y != graph->valueAxis()) {
        changed = true;
        res.y->setVisible(true);
        graph->setValueAxis(res.y);
        if (AppSettings::instance().autolitmAfterAxesChanged)
            _plot->autolimits(res.y, false);
    }
    if (changed) {
        _plot->replot();
        _diagram->markModified("PlotWindow::changeGraphAxes");
    }
}

void PlotWindow::settingsChanged()
{
    _plot->highlightAxesOfSelectedGraphs = AppSettings::instance().highlightAxesOfSelectedGraphs;
}
