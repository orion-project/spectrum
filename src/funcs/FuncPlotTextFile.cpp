#include "FuncPlotTextFile.h"

#include <QFile>
#include <QFileInfo>

bool FuncPlotTextFile::process()
{
    QFile f(_fileName);
    bool ok = f.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!ok)
    {
        _error = f.errorString();
        return false;
    }

    _text = f.readAll();

    ok = FuncPlotText::process();

    if (ok)
    {
        _title = QFileInfo(_fileName).fileName();
    }

    return ok;
}
