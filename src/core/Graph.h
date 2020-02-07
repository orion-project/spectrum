#ifndef GRAPH_H
#define GRAPH_H

#include <QString>
#include <QVector>
#include <QIcon>

#include "qcpl_types.h"

typedef QCPL::ValueArray Values;
typedef QCPL::GraphData GraphData;

class DataSource;
class Modificator;

class Graph
{
public:
    Graph(DataSource* dataSource);
    ~Graph();

    const QString& title() const { return _title; }
    void setTitle(const QString& title) { _title = title; _autoTitle = false; }

    const QColor& color() const { return _color; }
    void setColor(const QColor& color) { _color = color; }

    const QIcon& icon() const { return _icon; }
    void setIcon(const QIcon& icon) { _icon = icon; }

    const Values& x() { return _xs; }
    const Values& y() { return _ys; }
    void setData(const GraphData& data) { /*_data = data;*/ }

    QString canRefreshData() const;
    QString refreshData();

    /// The graph takes ownership on the modificator.
    QString modify(Modificator* mod);

private:
    bool _autoTitle = true;
    DataSource* _dataSource;
    QVector<double> _xs, _ys;
    QList<Modificator*> _modificators;
    //GraphData _data;
    QString _title;
    QIcon _icon;
    QColor _color;
};


class PlotObj
{
public:
    const QString& title() const { return _title; }
    const QIcon& icon() const { return _icon; }

private:
    QString _title;
    QIcon _icon;

    friend class PlotWindow;
};

#endif // GRAPH_H
