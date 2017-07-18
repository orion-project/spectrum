#ifndef GRAPHBUILDER_H
#define GRAPHBUILDER_H

#include <QString>

#include "Graph.h"

class GraphBuilder
{
public:
    static Graph* makeRandomSample();
    static Graph* makeFromTextData(const QString& text);

private:
    static Graph* makeGraph(const Values& x, const Values& y);
};

#endif // GRAPHBUILDER_H
