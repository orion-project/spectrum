#include "BaseTypes.h"

#include <QApplication>
#include <QJsonObject>

//------------------------------------------------------------------------------
//                              CsvGraphParams
//------------------------------------------------------------------------------

void CsvGraphParams::save(QJsonObject &root) const
{
    root["title"] = title;
    root["valueSeparators"] = valueSeparators;
    root["decimalPoint"] = decimalPoint;
    root["columnX"] = columnX;
    root["columnY"] = columnY;
    root["skipFirstLines"] = skipFirstLines;
}

//------------------------------------------------------------------------------
//                               PlottingRange
//------------------------------------------------------------------------------

QVector<double> PlottingRange::calcValues() const
{
    double range = stop - start;
    Q_ASSERT(range > 0);
    double step;
    int points;
    if (useStep)
    {
        // int((10.0 - 0.0) / 1.0) + 1 = 11 points
        // int((10.0 - 0.0) / 3.0) + 1 = 4 points: 0 3 6 9 10
        step = qMin(this->step, range);
        Q_ASSERT(step > 0);
        points = int(range / step) + 1;
    }
    else
    {
        Q_ASSERT(this->points > 1);
        // (10 - 0) / (10 - 1) ~ 1.1
        points = this->points;
        step = range / double(points - 1);
    }

    // Calc all point values
    double x = start;
    auto values = QVector<double>(points);
    for (int i = 0; i < points; i++)
    {
        values[i] = x;
        x += step;
    }
    return values;
}

QString PlottingRange::verify() const
{
    if (start >= stop)
        return qApp->tr("Start value must be less than stop value.");

    if (useStep && step <= 0)
        return qApp->tr("Numbers of steps must be greater than zero.");

    if (useStep && step >= (stop - start))
        return qApp->tr("Step must be less than the variation range.");

    if (!useStep && points < 2)
        return qApp->tr("Too few points");

    return QString();
}

void PlottingRange::save(QJsonObject &root) const
{
    root["start"] = start;
    root["stop"] = stop;
    root["step"] = step;
    root["points"] = points;
    root["useStep"] = useStep;
}

//------------------------------------------------------------------------------
//                               MinMax
//------------------------------------------------------------------------------

void MinMax::save(QJsonObject &root) const
{
    root["min"] = min;
    root["max"] = max;
}

//------------------------------------------------------------------------------
//                         RandomSampleParams
//------------------------------------------------------------------------------

void RandomSampleParams::save(QJsonObject &root) const
{
    QJsonObject x, y;
    rangeX.save(x);
    rangeY.save(y);
    root["rangeX"] = x;
    root["rangeY"] = y;
}