#include "FuncMakeFromClipboard.h"

#include <QApplication>
#include <QClipboard>

bool FuncMakeFromClipboard::process()
{
    _text = qApp->clipboard()->text();
    if (_text.isEmpty())
    {
        _error = qApp->tr("Clipboard does not contain suitable data.");
        return false;
    }

    return FuncMakeFromText::process();
}
