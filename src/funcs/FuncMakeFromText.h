#ifndef FuncMakeFromText_H
#define FuncMakeFromText_H

#include "FuncBase.h"

class FuncMakeFromText : public FuncBase
{
public:
    FuncMakeFromText(const QString& text) : _text(text) {}

    bool process() override;

protected:
    QString _text;
};

#endif // FuncMakeFromText_H
