#include "GraphBuilder.h"

Graph* GraphBuilder::makeGraph(const Values& x, const Values& y)
{
    auto g = new Graph;
    g->_x = x;
    g->_y = y;
    return g;
}

Graph* GraphBuilder::makeRandomSample()
{
    Values xs, ys;

    const double H = 25;
    const int count = 100;

    xs.resize(count);
    ys.resize(count);

    double y = (qrand()%100)*H*0.01;
    for (int i = 0; i < count; i++)
    {
        y = qAbs(y + (qrand()%100)*H*0.01 - H*0.5);

        xs.push_back(i);
        ys.push_back(y);
    }

    return makeGraph(xs, ys);
}

Graph* GraphBuilder::makeFromTextData(const QString& text)
{
    Q_UNUSED(text);
    return nullptr;
}
