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

struct PlottingRange
{
    double start;
    double stop;
    double step;
    int points = 100;
    bool useStep = false;

    QVector<double> calcValues() const;
    QString verify() const;
};

struct MinMax
{
    double min;
    double max;
};

struct RandomSampleParams
{
    PlottingRange rangeX;
    MinMax rangeY;
};

#define MSG_PLOT_RENAMED 1
#define MSG_GRAPH_RENAMED 2
#define MSG_GRAPH_DELETED 3
#define MSG_AXIS_FACTOR_CHANGED 4
#define MSG_PLOT_DELETED 5

#endif // BASE_TYPES_H
