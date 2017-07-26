#ifndef FuncPlotTextClipboard_H
#define FuncPlotTextClipboard_H

#include "FuncPlotText.h"

class FuncPlotTextClipboard : public FuncPlotText
{
public:
    FuncPlotTextClipboard() : FuncPlotText(QString()) {}

    bool process() override;
};

#endif // FuncPlotTextClipboard_H
