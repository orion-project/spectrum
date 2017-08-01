#include "FuncRandomSample.h"

//RandomSampleParams::RandomSampleParams() : QObject() {}

QString makeRandomSampleTitle()
{
    static int sampleCount = 0;
    return QString("random-sample (%1)").arg(++sampleCount);
}

bool FuncRandomSample::process()
{
    _title = makeRandomSampleTitle();

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

    return true;
}

//-----------------------------------------------------------------------------

class RandomSampleParamsEditor : public FuncParamsEditorBase
{
public:
    void populate(QObject* params) override {}
    void collect(QObject* params) override {}
};

//-----------------------------------------------------------------------------

bool FuncRandomSampleWithParams::process()
{
    _title = makeRandomSampleTitle();

    return true;
}

FuncParams* FuncRandomSampleWithParams::makeParams() { return (FuncParams*)new FuncRandomSampleParams; }
FuncParamsEditorBase* FuncRandomSampleWithParams::makeParamsEditor() { return new RandomSampleParamsEditor; }
