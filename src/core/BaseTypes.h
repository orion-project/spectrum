#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include "core/OriResult.h"

#include <QVector>

using Values = QVector<double>;

struct GraphPoints
{
    Values xs;
    Values ys;

    int size() const { return xs.size(); }
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

enum BusEvent
{
    ProjectModified,
    DiagramAdded,
    DiagramDeleted,
    DiagramRenamed,
    GraphAdded,
    GraphDeleting,
    GraphDeleted,
    GraphUpdated,
    GraphRenamed,
};

#endif // BASE_TYPES_H
