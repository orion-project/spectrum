#ifndef DATAGRIDPANEL_H
#define DATAGRIDPANEL_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class Graph;
class PlotObj;

namespace QCPL {
class GraphDataGrid;
}

class DataGridPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DataGridPanel(QWidget *parent = 0);

    void showData(PlotObj* plot, Graph* graph);

private:
    QLabel *_titlePlot, *_titleGraph;
    QLabel *_iconPlot, *_iconGraph;
    QCPL::GraphDataGrid* _dataGrid;
};

#endif // DATAGRIDPANEL_H
