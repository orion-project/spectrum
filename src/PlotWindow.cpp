#include "PlotWindow.h"
#include "qcpl_plot.h"
#include "helpers/OriLayouts.h"

PlotWindow::PlotWindow(QWidget *parent) : QWidget(parent)
{
    _plot = new QCPL::Plot;

    Ori::Layouts::LayoutV({_plot}).setMargin(0).setSpacing(0).useFor(this);
}
