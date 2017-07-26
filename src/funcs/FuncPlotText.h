#ifndef FuncPlotText_H
#define FuncPlotText_H

#include "FuncBase.h"

class FuncPlotText : public FuncBase
{
public:
    FuncPlotText(const QString& text) : _text(text) {}

    bool process() override;

protected:
    QString _text;
};

#endif // FuncPlotText_H
