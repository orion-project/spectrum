#ifndef FuncRandomSample_H
#define FuncRandomSample_H

#include "FuncBase.h"

FUNC_PARAMS(RandomSampleParams)
    FUNC_PARAM(double, xMin)
    FUNC_PARAM(double, xMax)
    FUNC_PARAM(double, yMin)
    FUNC_PARAM(double, yMax)
    FUNC_PARAM(double, step)
    FUNC_PARAM(int, points)
    FUNC_PARAM(bool, usePoints)
FUNC_PARAMS_END

class FuncRandomSample : public FuncBase
{
public:
    bool process() override;
};

#endif // FuncRandomSample_H
