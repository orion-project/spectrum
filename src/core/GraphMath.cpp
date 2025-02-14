#include "GraphMath.h"

#include <limits>

#include <QDebug>
#include <QtMath>

#define NEED_POINTS(cnt) \
    if (data.xs.size() != data.ys.size()) return data; \
    if (data.xs.size() < cnt) return data;

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

double min(const Values& data)
{
    double res = std::numeric_limits<double>::max();
    for (const double& v : data)
        if (v < res) res = v;
    return res;
}

double max(const Values& data)
{
    double res = -std::numeric_limits<double>::max();
    for (const double& v : data)
        if (v > res) res = v;
    return res;
}

double avg(const Values& data)
{
    double res = 0;
    for (const double& v : data)
        res += v;
    return res / double(data.size());
}

double mid(const Values& data)
{
    return (min(data) + max(data)) / 2.0;
}

GraphPoints Offset::calc(const GraphPoints& data) const
{
    NEED_POINTS(0)
    bool alongX = dir == DIR_X;
    const Values& values = alongX ? data.xs : data.ys;
    int count = values.size();
    Values newValues(count);
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

GraphPoints Reflect::calc(const GraphPoints& data) const
{
    NEED_POINTS(0)
    bool alongX = dir == DIR_X;
    const Values& values = alongX ? data.xs : data.ys;
    int count = values.size();
    Values newValues(count);
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

GraphPoints Flip::calc(const GraphPoints& data) const
{
    NEED_POINTS(0)
    bool alongX = dir == DIR_X;
    const Values& values = alongX ? data.xs : data.ys;
    int count = values.size();
    Values newValues(count);
    for (int i = 0; i < count; i++)
        newValues[i] = value - values.at(i);
    return {alongX ? newValues : data.xs, alongX ? data.ys : newValues};
}

GraphPoints Scale::calc(const GraphPoints& data) const
{
    NEED_POINTS(0)
    bool alongX = dir == DIR_X;
    const Values& values = alongX ? data.xs : data.ys;
    int count = values.size();
    Values newValues(count);
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
    NEED_POINTS(0)
    bool alongX = dir == DIR_X;
    const Values& values = alongX ? data.xs : data.ys;
    int count = values.size();
    Values newValues(count);
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
    NEED_POINTS(0)
    bool alongX = dir == DIR_X;
    const Values& values = alongX ? data.xs : data.ys;
    int count = values.size();
    Values newValues(count);
    for (int i = 0; i < count; i++)
        newValues[i] = value / values.at(i);
    return {alongX ? newValues : data.xs, alongX ? data.ys : newValues};
}

GraphPoints Decimate::calc(const GraphPoints& data) const
{
    NEED_POINTS(2)
    Values xs, ys;
    xs << data.xs.first();
    ys << data.ys.first();
    if ((useStep && step >= data.xs.last() - data.xs.first()) || points >= data.xs.size()) {
        xs << data.xs.last();
        ys << data.ys.last();
        return {xs, ys};
    }
    if ((useStep && step <= 0) || points <= 1)
        return {data.xs, data.ys};
    if (useStep) {
        double startX = data.xs.at(0);
        for (int i = 1; i < data.xs.size(); i++) {
            double x = data.xs.at(i);
            if (x - startX > step) {
                double prevX = data.xs.at(i-1);
                xs << prevX;
                ys << data.ys.at(i-1);
                startX = prevX;
            }
        }
    } else {
        int startI = 0;
        for (int i = 1; i < data.xs.size(); i++) {
            if (i - startI >= points) {
                xs << data.xs.at(i);
                ys << data.ys.at(i);
                startI = i;
            }
        }
    }
    return {xs, ys};
}

GraphPoints Average::calc(const GraphPoints& data) const
{
    NEED_POINTS(2)
    Values xs, ys;
    if ((useStep && step >= data.xs.last() - data.xs.first()) || points >= data.xs.size()) {
        double v = avg(data.ys);
        xs << data.xs.first();
        ys << v;
        xs << data.xs.last();
        ys << v;
        return {xs, ys};
    }
    if ((useStep && step <= 0) || points <= 1)
        return {data.xs, data.ys};
    if (useStep) {
        double startX = data.xs.at(0);
        double acc = data.ys.at(0);
        double cnt = 1;
        for (int i = 1; i < data.xs.size(); i++) {
            double x = data.xs.at(i);
            if (x - startX > step) {
                double prevX = data.xs.at(i-1);
                switch (pointPos) {
                case POS_BEG: xs << startX; break;
                case POS_END: xs << prevX; break;
                case POS_MID: xs << (prevX + startX)/2.0; break;
                }
                ys << acc / cnt;
                acc = 0;
                cnt = 0;
                startX = prevX;
            } else {
                acc += data.ys.at(i);
                cnt++;
            }
        }
    } else {
        int startI = 0;
        double startX = data.xs.at(0);
        double acc = data.ys.at(0);
        double cnt = 1;
        for (int i = 1; i < data.xs.size(); i++) {
            double x = data.xs.at(i);
            if (i - startI >= points) {
                switch (pointPos) {
                case POS_BEG: xs << startX; break;
                case POS_END: xs << x; break;
                case POS_MID: xs << (x + startX)/2.0; break;
                }
                ys << acc / cnt;
                acc = 0;
                cnt = 0;
                startI = i;
                startX = x;
            } else {
                acc += data.ys.at(i);
                cnt++;
            }
        }
    }
    return {xs, ys};
}

GraphPoints MavgSimple::calc(const GraphPoints& data) const
{
    NEED_POINTS(2)
    Values xs, ys;
    int cnt = points;
    if (useStep) {
        cnt = qFloor(step /  (data.xs[1] - data.xs[0]));
    }
    double avg = 0;
    double firstI = 0;
    for (int i = 0; i < data.ys.size(); i++) {
        const double y = data.ys.at(i);
        if (i < cnt-1) {
            avg += y;
            continue;
        } else if (i == cnt-1) {
            avg += y;
            avg /= double(cnt);
        } else {
            avg += (y - data.ys.at(firstI)) / double(cnt);
            firstI++;
        }
        xs << data.xs.at(i);
        ys << avg;
    }
    return {data.xs, ys};
}

GraphPoints MavgCumul::calc(const GraphPoints& data) const
{
    NEED_POINTS(1)
    Values ys(data.size());
    ys[0] = data.ys.at(0);
    double avg = ys[0];
    for (int i = 1; i < data.ys.size(); i++) {
        ys[i] = avg = (data.ys[i] + avg * double(i)) / double(i+1);
    }
    return {data.xs, ys};
}

GraphPoints MavgExp::calc(const GraphPoints& data) const
{
    NEED_POINTS(2)
    Values ys(data.size());
    ys[0] = data.ys.at(0);
    double avg = ys[0];
    for (int i = 1; i < data.ys.size(); i++) {
        ys[i] = data.ys[i] * alpha + ys[i-1] * (1.0 - alpha);
    }
    return {data.xs, ys};
}

GraphPoints FitLimits::calc(const GraphPoints& data) const
{
    NEED_POINTS(2)
    bool alongX = dir == DIR_X;
    const Values& values = alongX ? data.xs : data.ys;
    int count = values.size();
    Values newValues(count);
    double scale, oldOffset;
    if (alongX) {
        scale = qAbs(end - beg) / qAbs(values.last() - values.first());
        oldOffset = values.first();
    } else {
        double _min = min(data.ys);
        double _max = max(data.ys);
        scale = qAbs(end - beg) / (_max - _min);
        oldOffset = _min;
    }
    double newOffset = beg;
    for (int i = 0; i < count; i++)
        newValues[i] = (values.at(i) - oldOffset) * scale + newOffset;
    return {alongX ? newValues : data.xs, alongX ? data.ys : newValues};
}

GraphPoints Despike::calc(const GraphPoints& data) const
{
    NEED_POINTS(0)
    Values ys(data.ys.size());
    QVector<int> indexes;
    for (int i = 0; i < ys.size(); i++)
        indexes << i;
    int i = indexes.takeAt(rand() % indexes.size());
    double avg = data.ys.at(i);
    ys[i] = avg;
    if (mode == MODE_REL) {
        const double min = 1.0 - this->min / 100.0;
        const double max = 1.0 + this->max / 100.0;
        while (indexes.size() > 0) {
            auto i = indexes.takeAt(rand() % indexes.size());
            auto v = data.ys.at(i);
            if (v < avg * min || v > avg * max) {
                ys[i] = avg;
            } else {
                ys[i] = v;
                avg = (avg + v) / 2.0;
            }
        }
    } else {
        while (indexes.size() > 0) {
            auto i = indexes.takeAt(rand() % indexes.size());
            auto v = data.ys.at(i);
            // Skip cases when min-max are erroneously given
            // as laying totally outside of the graph trend
            if (avg >= min && avg <= max) {
                if (v < min || v > max) {
                    ys[i] = avg;
                } else {
                    ys[i] = v;
                    avg = (avg + v) / 2.0;
                }
            } else {
                ys[i] = v;
            }
        }
    }
    return {data.xs, ys};
}

GraphPoints Derivative::calc(const GraphPoints& data) const
{
    NEED_POINTS(2)
    const auto &x = data.xs;
    const auto &y = data.ys;
    Values dy(y.size());
    dy[0] = (y[1] - y[0]) / (x[1] - x[0]);
    for (int i = 1; i < x.size()-1; i++) {
        dy[i] = ((y[i+1] - y[i]) / (x[i+1] - x[i]) +
            (y[i] - y[i-1]) / (x[i] - x[i-1])) / 2.0;
    }
    int i = x.size() - 1;
    dy[i] = (y[i] - y[i-1]) / (x[i] - x[i-1]);
    return {x, dy};
}

} // namespace GraphMath
