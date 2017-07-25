#ifndef GRAPH_H
#define GRAPH_H

#include <QString>
#include <QVector>
#include <QIcon>

#include "qcpl_types.h"

typedef QCPL::ValueArray Values;
typedef QCPL::GraphData GraphData;

class Graph
{
public:
    const QString& title() const { return _title; }
    void setTitle(const QString& title) { _title = title; }

    const QColor& color() const { return _color; }
    void setColor(const QColor& color) { _color = color; }

    const QIcon& icon() const { return _icon; }
    void setIcon(const QIcon& icon) { _icon = icon; }

    const Values& x() { return _data.x; }
    const Values& y() { return _data.y; }
    void setData(const GraphData& data) { _data = data; }

private:
    GraphData _data;
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
