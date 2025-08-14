#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include "core/OriResult.h"
#include "tools/OriMessageBus.h"

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

#define BUS_EVENT(name) \
    struct name { \
        static const int id = __COUNTER__; \
        static void send(const QMap<QString, QVariant>& params = {}) {\
            Ori::MessageBus::send(id, params); \
        } \
    };
struct BusEvent
{
    BUS_EVENT(ErrorMessage)
    BUS_EVENT(ProjectModified)
    BUS_EVENT(ProjectUnmodified)
    BUS_EVENT(DiagramAdded)
    BUS_EVENT(DiagramDeleted)
    BUS_EVENT(DiagramRenamed)
    BUS_EVENT(DiagramFormatLoaded)
    BUS_EVENT(DiagramLoaded)
    BUS_EVENT(GraphAdded)
    BUS_EVENT(GraphLoaded)
    BUS_EVENT(GraphDeleting)
    BUS_EVENT(GraphDeleted)
    BUS_EVENT(GraphUpdated)
    BUS_EVENT(GraphRenamed)
};
#undef BUS_EVENT

#endif // BASE_TYPES_H
