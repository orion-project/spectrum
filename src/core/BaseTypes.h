#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include "core/OriResult.h"

#include <QVector>

class QJsonObject;

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
    
    void save(QJsonObject &root) const;
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
    
    void save(QJsonObject &root) const;
};

struct MinMax
{
    double min;
    double max;
    
    void save(QJsonObject &root) const;
};

struct RandomSampleParams
{
    PlottingRange rangeX;
    MinMax rangeY;
    
    void save(QJsonObject &root) const;
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
