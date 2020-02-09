#include "DataSources.h"

#include "../CustomPrefs.h"

#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>

namespace {

const QVector<QString>& valueSeparators()
{
    // TODO: make configurable
    static QVector<QString> separators({" ", "\t", ";", ","});
    return separators;
}

QString selectValueSeparator(const QStringRef& line)
{
    for (const QString& separator : valueSeparators())
    {
        QVector<QStringRef> parts = line.split(separator, QString::SkipEmptyParts);
        if (parts.size() > 1)
            return separator;
    }
    return QString();
}

GraphResult readDataFromText(const QString& text)
{
    if (text.isEmpty())
        return GraphResult::fail(qApp->tr("Processing text is empty."));

    QVector<QStringRef> lines = text.splitRef('\n', QString::SkipEmptyParts);
    if (lines.isEmpty())
        return GraphResult::fail(qApp->tr("Processing text is empty."));

    // It may be line of numbers, we can plot it spliting by a separator
    // 0.403922 0.419608 0.443137 0.458824 0.458824 0.466667 0.482353...
    if (lines.size() == 1)
    {
        for (const QString& separator : valueSeparators())
        {
            lines = text.splitRef(separator, QString::SkipEmptyParts);
            if (lines.size() >= 2)
                break;
        }
    }

    if (lines.size() < 2)
        // TODO try another line separator
        return GraphResult::fail(qApp->tr("Processing text contains too few lines."));

    bool ok, gotX, gotY;
    double value, x, y;
    QVector<double> xs, ys, onlyY;
//    bool hasValueSeparator = false;
//    bool hasOnlyOneColumn = false;
//    QString valueSeparator;

    for (const QStringRef& line : lines)
    {
        if (line.isEmpty()) continue;

        /*if (!hasValueSeparator)
        {
            valueSeparator = selectValueSeparator(line);
            if (valueSeparator.isEmpty())
            {
                // Try if the whole line is a single value
                value = line.toDouble(&ok);
                if (ok)
                {
                    hasValueSeparator = true;
                    hasOnlyOneColumn = true;
                }
                else continue;
            }
            // If `valueSeparator` is not empty, stil don't set `hasValueSeparator`
            // until we'll be sure it really separates values but not some header words
        }*/

        /*if (hasValueSeparator && hasOnlyOneColumn)
        {
            value = line.toDouble(&ok);
            if (ok)
                onlyY.push_back(value);
            continue;
        }*/


        QVector<QStringRef> parts;
        for (const QString& valueSeparator : valueSeparators())
        {
            parts = line.split(valueSeparator, QString::SkipEmptyParts);
            if (parts.size() > 1) break;
        }

        gotX = gotY = false;
        for (const QStringRef& part : parts)
        {
            value = part.toDouble(&ok);
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
            gotY = true;
        }
        else
        {
            xs.push_back(x);
            ys.push_back(y);
        }
        //if (!hasValueSeparator && (gotX || gotY))
          //  hasValueSeparator = true;
    }

    if (ys.size() < 2 && onlyY.size() < 2)
        return GraphResult::fail(qApp->tr("Too few points for plotting."));

    if (onlyY.size() > ys.size())
    {
        // treat data as single column
        ys = onlyY;
        xs.resize(ys.size());
        for (int i = 0; i < xs.size(); i++)
            xs[i] = i;
    }

    Q_ASSERT(xs.size() == ys.size());
    return GraphResult::ok({xs, ys});
}

} // namespace

//------------------------------------------------------------------------------
//                                 DataSource
//------------------------------------------------------------------------------

DataSource::~DataSource()
{
}

//------------------------------------------------------------------------------
//                             TextFileDataSource
//------------------------------------------------------------------------------

TextFileDataSource::TextFileDataSource()
{
}

bool TextFileDataSource::configure()
{
    auto root = CustomDataHelpers::loadCustomData("datasources");
    auto state = root["file"].toObject();

    QFileDialog dlg(qApp->activeWindow());
    if (_fileName.isEmpty())
    {
        auto dir = state["dir"].toString();
        if (!dir.isEmpty())
            dlg.setDirectory(dir);
    }
    else
        dlg.selectFile(_fileName);

    if (dlg.exec() == QDialog::Accepted)
    {
        auto files = dlg.selectedFiles();
        if (files.isEmpty()) return false;
        auto fileName = files.first();
        if (fileName.isEmpty()) return false;
        _fileName = fileName;

        state["dir"] = dlg.directory().path();
        root["file"] = state;
        CustomDataHelpers::saveCustomData(root, "datasources");

        return true;
    }
    return false;
}

GraphResult TextFileDataSource::getData() const
{
    QFile f(_fileName);
    if (!f.exists())
        return GraphResult::fail(qApp->tr("File '%1' not found").arg(_fileName));

    bool ok = f.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!ok)
        return GraphResult::fail(qApp->tr("Failed to read file '%1': %2").arg(_fileName, f.errorString()));

    return readDataFromText(f.readAll());
}

QString TextFileDataSource::makeTitle() const
{
    return QFileInfo(_fileName).fileName();
}

//------------------------------------------------------------------------------
//                             RandomSampleDataSource
//------------------------------------------------------------------------------

static int __randomSampleIndex = 0;

RandomSampleDataSource::RandomSampleDataSource()
{
    _index = ++__randomSampleIndex;
}

GraphResult RandomSampleDataSource::getData() const
{
    const double H = 25;
    const int count = 100;

    QVector<double> xs(count);
    QVector<double> ys(count);

    double y = (qrand()%100)*H*0.01;
    for (int i = 0; i < count; i++)
    {
        y = qAbs(y + (qrand()%100)*H*0.01 - H*0.5);

        xs[i] = i;
        ys[i] = y;
    }

    return GraphResult::ok({xs, ys});
}

QString RandomSampleDataSource::canRefresh() const
{
    return qApp->tr("Refreshing of random graph data is not supported");
}

QString RandomSampleDataSource::makeTitle() const
{
     return QString("random-sample (%1)").arg(_index);
}

//------------------------------------------------------------------------------
//                             ClipboardDataSource
//------------------------------------------------------------------------------

static int __clipboardCallCount = 0;

ClipboardDataSource::ClipboardDataSource()
{
    if (qApp->clipboard()->mimeData()->hasText())
        _index = ++__clipboardCallCount;
}

GraphResult ClipboardDataSource::getData() const
{
    QString text = qApp->clipboard()->text();
    if (text.isEmpty())
        return GraphResult::fail(qApp->tr("Clipboard does not contain suitable data"));

    return readDataFromText(text);
}

QString ClipboardDataSource::canRefresh() const
{
    return qApp->tr("Refreshing of graph data from the clipboard is not supported");
}

QString ClipboardDataSource::makeTitle() const
{
     return QString("clipboard (%1)").arg(_index);
}
