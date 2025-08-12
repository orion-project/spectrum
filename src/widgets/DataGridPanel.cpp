#include "DataGridPanel.h"

#include "core/Graph.h"
#include "core/Project.h"

#include "helpers/OriLayouts.h"

#include "qcpl_graph_grid.h"

#include <QLabel>
#include <QDebug>

using namespace Ori::Layouts;

DataGridPanel::DataGridPanel(Project *project, QWidget *parent)
    : QWidget(parent), Ori::IMessageBusListener(), _project(project)
{
    _dataGrid = new QCPL::GraphDataGrid;
    _iconPlot = new QLabel;
    _iconGraph = new QLabel;
    _titlePlot = new QLabel("<span style='color:gray'>no diagram selected</span>");
    _titleGraph = new QLabel("<span style='color:gray'>no graph selected</span>");

    LayoutV({
                LayoutH({_iconPlot, _titlePlot, Stretch()}).setMargin(3).setSpacing(6),
                LayoutH({_iconGraph, _titleGraph, Stretch()}).setMargin(3).setSpacing(6),
                _dataGrid
            })
            .setMargin(0)
            .setSpacing(0)
            .useFor(this);
}

void DataGridPanel::messageBusEvent(int event, const QMap<QString, QVariant>& params)
{
    switch (event) {
    case BusEvent::DiagramRenamed:
        if (isVisible() && params.value("id") == _plotId)
            showData(_project->diagram(_plotId), nullptr);
        break;
    }
}

void DataGridPanel::showData(Diagram *dia, Graph *graph)
{
    if (dia)
    {
        _plotId = dia->id();
        _iconPlot->setPixmap(dia->icon().pixmap(16, 16));
        _titlePlot->setText(dia->title());
    }
    if (graph)
    {
        _graphId = graph->id();
        _iconGraph->setPixmap(graph->icon().pixmap(16, 16));
        _titleGraph->setText(graph->title());
        _dataGrid->setData(graph->data().xs, graph->data().ys);
    }
}

void DataGridPanel::copyData()
{
    _dataGrid->copy();
}

bool DataGridPanel::hasFocus() const
{
    return _dataGrid->hasFocus();
}
