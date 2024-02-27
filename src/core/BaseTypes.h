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

#define MSG_PLOT_RENAMED 1
#define MSG_GRAPH_RENAMED 2
#define MSG_GRAPH_DELETED 3
#define MSG_AXIS_FACTOR_CHANGED 4

#endif // BASE_TYPES_H
