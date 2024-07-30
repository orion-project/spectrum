#include "GraphMath.h"

#include <limits>

namespace GraphMath {

MinMax minMax(const GraphPoints& data)
{
    int count = data.xs.size();
    Q_ASSERT(count == data.ys.size());
    MinMax res;
    res.minX = std::numeric_limits<double>::max();
    res.maxX = -std::numeric_limits<double>::max();
    res.minY.y = std::numeric_limits<double>::max();
    res.minY.x = Q_QNAN;
    res.minY.index = -1;
    res.maxY.y = -std::numeric_limits<double>::max();
    res.maxY.x = Q_QNAN;
    res.maxY.index = -1;
    for (int i = 0; i < count; i++)
    {
        auto x = data.xs.at(i);
        auto y = data.ys.at(i);
        if (x > res.maxX) res.maxX = x;
        if (x < res.minX) res.minX = x;
        if (y > res.maxY.y)
        {
            res.maxY.y = y;
            res.maxY.x = x;
            res.maxY.index = i;
        }
        if (y < res.minY.y)
        {
            res.minY.y = y;
            res.minY.x = x;
            res.minY.index = i;
        }
    }
    return res;
}

double min(const QVector<double>& data)
{
    double res = std::numeric_limits<double>::max();
    for (const double& v : data)
        if (v < res) res = v;
    return res;
}

double max(const QVector<double>& data)
{
    double res = -std::numeric_limits<double>::max();
    for (const double& v : data)
        if (v > res) res = v;
    return res;
}

double avg(const QVector<double>& data)
{
    double res = 0;
    for (const double& v : data)
        res += v;
    return res / double(data.size());
}

double mid(const QVector<double>& data)
{
    return (min(data) + max(data)) / 2.0;
}

GraphPoints Offset::calc(const GraphPoints& data) const
{
    bool alongX = dir == DIR_X;
    const QVector<double>& values = alongX ? data.xs : data.ys;
    int count = data.ys.size();
    QVector<double> newValues(count);
    double offset = 0;
    switch(mode)
    {
    case MODE_MAX: offset = -max(values); break;
    case MODE_MIN: offset = -min(values); break;
    case MODE_AVG: offset = -avg(values); break;
    case MODE_MID: offset = -mid(values); break;
    case MODE_VAL: offset = value; break;
    }
    for (int i = 0; i < count; i++)
        newValues[i] = values.at(i) + offset;
    return {alongX ? newValues : data.xs, alongX ? data.ys : newValues};
}

GraphPoints Flip::calc(const GraphPoints& data) const
{
    bool alongX = dir == DIR_X;
    const QVector<double>& values = alongX ? data.xs : data.ys;
    int count = data.ys.size();
    QVector<double> newValues(count);
    double center = 0;
    switch(centerMode)
    {
    case CENTER_ZERO: center = 0; break;
    case CENTER_MAX: center = max(values); break;
    case CENTER_MIN: center = min(values); break;
    case CENTER_AVG: center = avg(values); break;
    case CENTER_MID: center = mid(values); break;
    case CENTER_VAL: center = centerValue; break;
    }
    center *= 2; // -(g - c) + c
    for (int i = 0; i < count; i++)
        newValues[i] = center - values.at(i);
    return {alongX ? newValues : data.xs, alongX ? data.ys : newValues};
}

GraphPoints FlipRaw::calc(const GraphPoints& data) const
{
    bool alongX = dir == DIR_X;
    const QVector<double>& values = alongX ? data.xs : data.ys;
    int count = data.ys.size();
    QVector<double> newValues(count);
    for (int i = 0; i < count; i++)
        newValues[i] = value - values.at(i);
    return {alongX ? newValues : data.xs, alongX ? data.ys : newValues};
}

GraphPoints Scale::calc(const GraphPoints& data) const
{
    bool alongX = dir == DIR_X;
    const QVector<double>& values = alongX ? data.xs : data.ys;
    int count = data.ys.size();
    QVector<double> newValues(count);
    double offset = 0;
    switch (centerMode)
    {
    case CENTER_ZERO: offset = 0; break;
    case CENTER_MAX: offset = max(values); break;
    case CENTER_MIN: offset = min(values); break;
    case CENTER_AVG: offset = avg(values); break;
    case CENTER_MID: offset = mid(values); break;
    case CENTER_VAL: offset = centerValue;
    }
    for (int i = 0; i < count; i++)
        newValues[i] = (values.at(i) - offset) * scaleFactor + offset;
    return {alongX ? newValues : data.xs, alongX ? data.ys : newValues};
}

GraphPoints Normalize::calc(const GraphPoints& data) const
{
    bool alongX = dir == DIR_X;
    const QVector<double>& values = alongX ? data.xs : data.ys;
    int count = data.ys.size();
    QVector<double> newValues(count);
    double factor = 1;
    switch (mode)
    {
    case MODE_MAX: factor = max(values); break;
    case MODE_VAL: factor = value;
    }
    for (int i = 0; i < count; i++)
        newValues[i] = values.at(i) / factor;
    return {alongX ? newValues : data.xs, alongX ? data.ys : newValues};
}

GraphPoints Invert::calc(const GraphPoints& data) const
{
    bool alongX = dir == DIR_X;
    const QVector<double>& values = alongX ? data.xs : data.ys;
    int count = data.ys.size();
    QVector<double> newValues(count);
    for (int i = 0; i < count; i++)
        newValues[i] = value / values.at(i);
    return {alongX ? newValues : data.xs, alongX ? data.ys : newValues};
}

} // namespace GraphMath
