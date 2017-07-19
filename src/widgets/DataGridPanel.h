#ifndef DATAGRIDPANEL_H
#define DATAGRIDPANEL_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class Graph;
class GraphDataGrid;

class DataGridPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DataGridPanel(QWidget *parent = 0);

    void showData(const QString& plotTitle, Graph* g);

private:
    QLabel *_titlePlot, *_titleGraph;
    GraphDataGrid* _dataGrid;
};

#endif // DATAGRIDPANEL_H
