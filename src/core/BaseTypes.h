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

struct CsvGraphParams
{
    QString title;
    QString valueSeparators;
    bool decimalPoint;
    int columnX, columnY;
    int skipFirstLines;
};

#endif // BASE_TYPES_H
