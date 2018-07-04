#ifndef FUNCRANDOMSAMPLEPARAMS_H
#define FUNCRANDOMSAMPLEPARAMS_H

#include "FuncBase.h"

//class FuncRandomSampleParams : public QObject
//{
//    Q_OBJECT
//public:
//    explicit FuncRandomSampleParams(QObject *parent = 0);

//signals:

//public slots:
//};

FUNC_PARAMS(FuncRandomSampleParams)
Q_OBJECT
public:
    FuncRandomSampleParams();
    FUNC_PARAM(double, xMin, 0)
    FUNC_PARAM(double, xMax, 100)
    FUNC_PARAM(double, yMin, 0)
    FUNC_PARAM(double, yMax, 100)
    FUNC_PARAM(double, step, 1)
    FUNC_PARAM(int, points, 101)
    FUNC_PARAM(bool, usePoints, false)
FUNC_PARAMS_END

#endif // FUNCRANDOMSAMPLEPARAMS_H
