#ifndef GRAPH_MATH_H
#define GRAPH_MATH_H

#include "BaseTypes.h"

class QJsonObject;

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

enum Direction {DIR_Y, DIR_X};

struct Offset
{
    Direction dir;
    enum Mode {MODE_MAX, MODE_MIN, MODE_AVG, MODE_MID, MODE_VAL} mode;
    double value;
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const;
    void load(const QJsonObject &obj);
};

struct Reflect
{
    Direction dir;
    enum Mode {CENTER_ZERO, CENTER_MAX, CENTER_MIN, CENTER_AVG, CENTER_MID, CENTER_VAL} centerMode;
    double centerValue;
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const;
    void load(const QJsonObject &obj);
};

struct Flip
{
    Direction dir;
    double value;
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const;
    void load(const QJsonObject &obj);
};

struct Scale
{
    Direction dir;
    enum CenterMode {CENTER_ZERO, CENTER_MAX, CENTER_MIN, CENTER_AVG, CENTER_MID, CENTER_VAL} centerMode;
    double centerValue;
    double scaleFactor;
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const;
    void load(const QJsonObject &obj);
};

struct Normalize
{
    Direction dir;
    enum Mode {MODE_MAX, MODE_VAL} mode;
    double value;
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const;
    void load(const QJsonObject &obj);
};

struct Invert
{
    Direction dir;
    double value;
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const;
    void load(const QJsonObject &obj);
};

struct Decimate
{
    int points;
    double step;
    bool useStep;
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const;
    void load(const QJsonObject &obj);
};

struct Average
{
    int points;
    double step;
    bool useStep;
    enum PointPos { POS_MID, POS_BEG, POS_END } pointPos;
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const;
    void load(const QJsonObject &obj);
};

struct MavgSimple
{
    int points;
    double step;
    bool useStep;
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const;
    void load(const QJsonObject &obj);
};

struct MavgCumul
{
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const {}
    void load(const QJsonObject &obj) {}
};

struct MavgExp
{
    double alpha;
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const;
    void load(const QJsonObject &obj);
};

struct FitLimits
{
    Direction dir;
    double beg, end;
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const;
    void load(const QJsonObject &obj);
};

struct Despike
{
    enum Mode {MODE_REL, MODE_ABS} mode;
    double min, max;
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const;
    void load(const QJsonObject &obj);
};

struct Derivative
{
    enum Mode { MODE_SIMPLE, MODE_REFINED, MODE_SIMPLE_TAU, MODE_REFINED_TAU } mode;
    double tau;
    GraphPoints calc(const GraphPoints& data) const;
    void save(QJsonObject &obj) const;
    void load(const QJsonObject &obj);
};

} // namespace GraphMath

#endif // GRAPH_MATH_H
