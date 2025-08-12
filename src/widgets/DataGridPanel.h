#ifndef DATA_GRID_PANEL_H
#define DATA_GRID_PANEL_H

#include "tools/OriMessageBus.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class Project;
class Diagram;
class Graph;

namespace QCPL {
class GraphDataGrid;
}

class DataGridPanel : public QWidget, public Ori::IMessageBusListener
{
    Q_OBJECT

public:
    explicit DataGridPanel(Project *project, QWidget *parent = nullptr);

    // Ori::IMessageBusListener
    void messageBusEvent(int event, const QMap<QString, QVariant>& params) override;

    bool hasFocus() const;
    void showData(Diagram* dia, Graph* graph);
    void copyData();

    QString plotId() const { return _plotId; }
    QString graphId() const { return _graphId; }

private:
    Project *_project;
    QLabel *_titlePlot, *_titleGraph;
    QLabel *_iconPlot, *_iconGraph;
    QCPL::GraphDataGrid* _dataGrid;
    QString _plotId, _graphId;
};

#endif // DATA_GRID_PANEL_H
