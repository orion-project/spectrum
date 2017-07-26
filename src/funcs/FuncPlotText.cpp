#include "FuncPlotText.h"

#include <QApplication>

bool FuncPlotText::process()
{
    if (_text.isEmpty())
    {
        _error = qApp->tr("Processing text is empty.");
        return false;
    }

    QVector<QStringRef> lines = _text.splitRef('\n', QString::SkipEmptyParts);
    if (lines.size() < 2)
    {
        // TODO try another line separator
        _error = qApp->tr("Processing text contains too few lines.");
        return false;
    }

    bool ok, gotX, gotY;
    double value, x, y;
    QCPL::ValueArray xs, ys, onlyY;

    for (int i = 0; i < lines.size(); i++)
    {
        gotX = gotY = false;
        QVector<QStringRef> parts = lines.at(i).split('\t', QString::SkipEmptyParts);
        for (int j = 0; j < parts.size(); j++)
        {
            value = parts.at(j).toDouble(&ok);
            if (!ok)
            {
                // TODO try another decimal separator
                continue;
            }
            if (!gotX)
            {
                x = value;
                gotX = true;
            }
            else
            {
                y = value;
                gotY = true;
            }
            if (gotX && gotY) break;
        }
        if (!gotY)
        {
            if (!gotX)
            {
                // TODO try another value separator
                continue;
            }
            onlyY.push_back(x);
        }
        else
        {
            xs.push_back(x);
            ys.push_back(y);
        }
    }
    if (ys.size() < 2 && onlyY.size() < 2)
    {
        _error = qApp->tr("Too few points for plotting.");
        return false;
    }
    if (onlyY.size() > ys.size())
    {
        // treat data as single column
        ys = onlyY;
        xs.resize(ys.size());
        for (int i = 0; i < xs.size(); i++)
            xs[i] = i;
    }
    Q_ASSERT(xs.size() == ys.size());
    _data.x = xs;
    _data.y = ys;
    return true;
}
