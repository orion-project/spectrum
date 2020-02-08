#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include "core/OriResult.h"

#include <QVector>

struct GraphPoints
{
    QVector<double> xs;
    QVector<double> ys;
};

using GraphResult = Ori::Result<GraphPoints>;

#endif // BASE_TYPES_H
