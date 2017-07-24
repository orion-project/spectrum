#ifndef GRAPH_H
#define GRAPH_H

#include <QString>
#include <QVector>
#include <QIcon>

#include "qcpl_types.h"

typedef QCPL::ValueArray Values;

class Graph
{
public:
    const QString& title() const { return _title; }

    const QColor& color() const { return _color; }
    void setColor(const QColor& color) { _color = color; }

    const QIcon& icon() const { return _icon; }
    void setIcon(const QIcon& icon) { _icon = icon; }

    const Values& x() { return _x; }
    const Values& y() { return _y; }

private:
    Values _x, _y;
    QString _title;
    QIcon _icon;
    QColor _color;

    friend class GraphBuilder;
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
