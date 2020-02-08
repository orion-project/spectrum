#ifndef GRAPH_MATH_H
#define GRAPH_MATH_H

#include "BaseTypes.h"

namespace GraphMath {

struct MinMax
{
    struct Extremum
    {
        double x;
        double y;
        int index;
    };
    double minX, maxX;
    Extremum minY, maxY;
};

MinMax minMax(const GraphPoints& data);
double min(const QVector<double>& data);
double max(const QVector<double>& data);
double avg(const QVector<double>& data);

struct Offset
{
    enum Direction {DIR_X, DIR_Y} dir;
    enum Mode {MODE_MAX, MODE_MIN, MODE_AVG, MODE_VAL} mode;
    double value;
    GraphPoints calc(const GraphPoints& data) const;
};

struct Scale
{
    enum Direction {DIR_X, DIR_Y} dir;
    enum CenterMode {CENTER_NON, CENTER_MAX, CENTER_MIN, CENTER_AVG, CENTER_VAL} centerMode;
    double centerValue;
    double scaleFactor;
    GraphPoints calc(const GraphPoints& data) const;
};

} // namespace GraphMath

#endif // GRAPH_MATH_H
