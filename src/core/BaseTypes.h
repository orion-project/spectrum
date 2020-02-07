#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include "core/OriResult.h"

#include <QVector>

struct GraphPoints
{
    const QVector<double> xs;
    const QVector<double> ys;
};

using GraphResult = Ori::Result<GraphPoints>;

#endif // BASE_TYPES_H
