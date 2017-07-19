#ifndef GRAPH_H
#define GRAPH_H

#include <QString>
#include <QVector>

typedef QVector<double> Values;

class Graph
{
public:
    Graph();

    const QString& title() const { return _title; }

    const Values& x() { return _x; }
    const Values& y() { return _y; }

private:
    Values _x, _y;
    QString _title;

    friend class GraphBuilder;
};

#endif // GRAPH_H
