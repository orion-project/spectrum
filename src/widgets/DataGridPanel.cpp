#include "DataGridPanel.h"
#include "helpers/OriLayouts.h"
#include "qcpl_graph_grid.h"
#include "../core/Graph.h"

#include <QLabel>
#include <QDebug>

using namespace Ori::Layouts;

DataGridPanel::DataGridPanel(QWidget *parent) : QWidget(parent)
{
    _dataGrid = new QCPL::GraphDataGrid;
    _iconPlot = new QLabel;
    _iconGraph = new QLabel;
    _titlePlot = new QLabel;
    _titleGraph = new QLabel;

    LayoutV({
                LayoutH({_iconPlot, _titlePlot, Stretch()}).setMargin(3).setSpacing(6),
                LayoutH({_iconGraph, _titleGraph, Stretch()}).setMargin(3).setSpacing(6),
                _dataGrid
            })
            .setMargin(0)
            .setSpacing(0)
            .useFor(this);
}

void DataGridPanel::showData(PlotObj *plot, Graph *graph)
{
    _iconPlot->setPixmap(plot->icon().pixmap(16, 16));
    _titlePlot->setText(plot->title());
    _iconGraph->setPixmap(graph->icon().pixmap(16, 16));
    _titleGraph->setText(graph->title());
    _dataGrid->setData(graph->data().xs, graph->data().ys);
}

void DataGridPanel::copyData()
{
    _dataGrid->copy();
}

bool DataGridPanel::hasFocus() const
{
    return _dataGrid->hasFocus();
}
