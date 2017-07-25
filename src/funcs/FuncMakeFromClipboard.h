#ifndef FuncMakeFromClipboard_H
#define FuncMakeFromClipboard_H

#include "FuncMakeFromText.h"

class FuncMakeFromClipboard : public FuncMakeFromText
{
public:
    FuncMakeFromClipboard() : FuncMakeFromText(QString()) {}

    bool process() override;
};

#endif // FuncMakeFromClipboard_H
