#include "FuncPlotTextClipboard.h"

#include <QApplication>
#include <QClipboard>

bool FuncPlotTextClipboard::process()
{
    _text = qApp->clipboard()->text();
    if (_text.isEmpty())
    {
        _error = qApp->tr("Clipboard does not contain suitable data.");
        return false;
    }

    bool ok = FuncPlotText::process();

    if (ok)
    {
        static int callCount = 0;
        _title = QString("clipboard (%1)").arg(++callCount);
    }

    return ok;
}
