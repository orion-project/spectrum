#ifndef FuncRandomSample_H
#define FuncRandomSample_H

#include "FuncBase.h"
#include "FuncRandomSampleParams.h"

//FUNC_PARAMS(RandomSampleParams)
////Q_OBJECT
//    FUNC_PARAM(double, xMin, 0)
//    FUNC_PARAM(double, xMax, 100)
//    FUNC_PARAM(double, yMin, 0)
//    FUNC_PARAM(double, yMax, 100)
//    FUNC_PARAM(double, step, 1)
//    FUNC_PARAM(int, points, 101)
//    FUNC_PARAM(bool, usePoints, false)
//FUNC_PARAMS_END

//FUNC_PARAMS(RandomSampleParams)
//    Q_OBJECT
//    FUNC_PARAM(double, xMin, 0)
//FUNC_PARAMS_END

class FuncRandomSample : public FuncBase
{
public:
    bool process() override;
};


class FuncRandomSampleWithParams : public ConfigFuncBase
{
public:
    bool process() override;

protected:
    FuncParams* makeParams() override;
    FuncParamsEditorBase* makeParamsEditor() override;
};

#endif // FuncRandomSample_H
