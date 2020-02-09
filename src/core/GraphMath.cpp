#include "GraphMath.h"

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
    case MODE_VAL: offset = value; break;
    }
    for (int i = 0; i < count; i++)
        newValues[i] = values.at(i) + offset;
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
    case CENTER_NON: offset = 0; break;
    case CENTER_MAX: offset = GraphMath::max(values); break;
    case CENTER_MIN: offset = GraphMath::min(values); break;
    case CENTER_AVG: offset = GraphMath::avg(values); break;
    case CENTER_VAL: offset = centerValue;
    }
    for (int i = 0; i < count; i++)
        newValues[i] = (values.at(i) - offset) * scaleFactor + offset;
    return {alongX ? newValues : data.xs, alongX ? data.ys : newValues};
}

} // namespace GraphMath
