#ifndef GRAPH_H
#define GRAPH_H

#include <QVector>

typedef QVector<double> Values;

class Graph
{
public:
    Graph();

    const Values& x() { return _x; }
    const Values& y() { return _y; }

private:
    Values _x, _y;

    friend class GraphBuilder;
};

#endif // GRAPH_H
