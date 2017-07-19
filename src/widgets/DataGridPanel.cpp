#include "DataGridPanel.h"
#include "helpers/OriLayouts.h"
#include "GraphDataGrid.h"
#include "../core/Graph.h"

#include <QLabel>
#include <QDebug>

using namespace Ori::Layouts;

DataGridPanel::DataGridPanel(QWidget *parent) : QWidget(parent)
{
    _dataGrid = new GraphDataGrid;

    auto iconPlot = new QLabel;
    auto iconGraph = new QLabel;

    iconPlot->setPixmap(QIcon(":/icon/ball").pixmap(16, 16)); // TODO icon
    iconGraph->setPixmap(QIcon(":/icon/ball").pixmap(16, 16)); // TODO icon

    _titlePlot = new QLabel;
    _titleGraph = new QLabel;

    LayoutV({
                LayoutH({iconPlot, _titlePlot, Stretch()}).setMargin(3).setSpacing(6),
                LayoutH({iconGraph, _titleGraph, Stretch()}).setMargin(3).setSpacing(6),
                _dataGrid
            })
            .setMargin(0)
            .setSpacing(0)
            .useFor(this);
}

void DataGridPanel::showData(const QString& plotTitle, Graph* g)
{
    _titlePlot->setText(plotTitle);
    _titleGraph->setText(g->title());
    _dataGrid->setData(g->x(), g->y());
}
