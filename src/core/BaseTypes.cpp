#include "BaseTypes.h"

#include <QApplication>
#include <QJsonObject>

//------------------------------------------------------------------------------
//                              CsvGraphParams
//------------------------------------------------------------------------------

void CsvGraphParams::save(QJsonObject &obj) const
{
    obj["title"] = title;
    obj["valueSeparators"] = valueSeparators;
    obj["decimalPoint"] = decimalPoint;
    obj["columnX"] = columnX;
    obj["columnY"] = columnY;
    obj["skipFirstLines"] = skipFirstLines;
}

void CsvGraphParams::load(const QJsonObject &obj)
{
    title = obj["title"].toString();
    valueSeparators = obj["valueSeparators"].toString();
    decimalPoint = obj["decimalPoint"].toBool();
    columnX = obj["columnX"].toInt();
    columnY = obj["columnY"].toInt();
    skipFirstLines = obj["skipFirstLines"].toInt();
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

void PlottingRange::save(QJsonObject &obj) const
{
    obj["start"] = start;
    obj["stop"] = stop;
    obj["step"] = step;
    obj["points"] = points;
    obj["useStep"] = useStep;
}

void PlottingRange::load(const QJsonObject &obj)
{
    start = obj["start"].toDouble();
    stop = obj["stop"].toDouble();
    step = obj["step"].toDouble();
    points = obj["points"].toInt(100);
    useStep = obj["useStep"].toBool(false);
}

//------------------------------------------------------------------------------
//                               MinMax
//------------------------------------------------------------------------------

void MinMax::save(QJsonObject &obj) const
{
    obj["min"] = min;
    obj["max"] = max;
}

void MinMax::load(const QJsonObject &obj)
{
    min = obj["min"].toDouble(0);
    max = obj["max"].toDouble(1);
}

//------------------------------------------------------------------------------
//                         RandomSampleParams
//------------------------------------------------------------------------------

void RandomSampleParams::save(QJsonObject &obj) const
{
    QJsonObject x, y;
    rangeX.save(x);
    rangeY.save(y);
    obj["rangeX"] = x;
    obj["rangeY"] = y;
}

void RandomSampleParams::load(const QJsonObject &obj)
{
    rangeX.load(obj["rangeX"].toObject());
    rangeY.load(obj["rangeY"].toObject());
}
