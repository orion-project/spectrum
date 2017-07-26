#include "FuncRandomSample.h"

bool FuncRandomSample::process()
{
    static int sampleCount = 0;
    _title = QString("random-sample (%1)").arg(++sampleCount);

    const double H = 25;
    const int count = 100;

    _data.x.resize(count);
    _data.y.resize(count);

    double y = (qrand()%100)*H*0.01;
    for (int i = 0; i < count; i++)
    {
        y = qAbs(y + (qrand()%100)*H*0.01 - H*0.5);

        _data.x[i] = i;
        _data.y[i] = y;
    }

    RandomSampleParams p;
    p.xMin = 345;
    p.xMax = 222;
    p.yMin = 23;
    p.yMax = 1000;
    saveFuncParams("FuncRandomSample", &p);

    return true;
}
