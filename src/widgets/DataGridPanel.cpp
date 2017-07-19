#include "DataGridPanel.h"
#include "helpers/OriLayouts.h"
#include "../core/Graph.h"

#include <QLabel>
#include <QDebug>

using namespace Ori::Layouts;

DataGridPanel::DataGridPanel(QWidget *parent) : QWidget(parent)
{
    auto dataGrid = new QWidget;

    auto iconPlot = new QLabel("Plot:"); // TODO icon
    auto iconGraph = new QLabel("Graph:"); // TODO graph

    _titlePlot = new QLabel;
    _titleGraph = new QLabel;

    LayoutV({
                LayoutH({iconPlot, _titlePlot}).setMargin(3),
                LayoutH({iconGraph, _titleGraph}).setMargin(3),
                dataGrid,
                Stretch()
            })
            .setMargin(0)
            .setSpacing(0)
            .useFor(this);
}

void DataGridPanel::showData(const QString& plotTitle, Graph* g)
{
    _titlePlot->setText(plotTitle);
    _titleGraph->setText(g->title());
}
