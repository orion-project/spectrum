#include "Modificators.h"

//------------------------------------------------------------------------------
//                                 Modificator
//------------------------------------------------------------------------------

Modificator::~Modificator()
{
}

//------------------------------------------------------------------------------
//                              OffsetModificator
//------------------------------------------------------------------------------

GraphResult OffsetModificator::modify(const GraphPoints &data) const
{
    int count = data.ys.size();
    QVector<double> newY(count);

    for (int i = 0; i < count; i++)
        newY[i] = data.ys.at(i) * 0.5;

    return GraphResult::ok({data.xs, newY});
}
