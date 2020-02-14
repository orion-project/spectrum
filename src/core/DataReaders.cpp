#include "DataReaders.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>

static const QVector<QString>& valueSeparators()
{
    // TODO: make configurable
    static QVector<QString> separators({" ", "\t", ",", ";"});
    return separators;
}

//------------------------------------------------------------------------------
//                               LineSplitter
//------------------------------------------------------------------------------

void LineSplitter::detect(const QString& line)
{
    // Don't skip empty parts; a line like "1,,3" should issue 3 columns
    splitBehavior = QString::KeepEmptyParts;
    for (auto sep = separators.cbegin(); sep != separators.cend(); sep++)
    {
        separator = *sep;
        parts = line.splitRef(separator, splitBehavior);
        if (parts.size() > 1)
        {
            empty = false;
            return;
        }
    }
    // Skip empty parts; a line like "1    3" should issue only 2 columns
    splitBehavior = QString::SkipEmptyParts;
    separator = ' ';
    parts = line.splitRef(separator, splitBehavior);
    if (parts.size() > 1)
    {
        empty = false;
        return;
    }
    separator = '\t';
    parts = line.splitRef(separator, splitBehavior);
    if (parts.size() > 1)
    {
        empty = false;
        return;
    }
    return;
}

void LineSplitter::split(const QString& line)
{
    if (empty)
        detect(line);
    else
        parts = line.splitRef(separator, splitBehavior);
}

struct TextFileOpener
{
    TextFileOpener(const QString& fileName);

    QFile file;
    QString error;
};

//------------------------------------------------------------------------------
//                                  CsvMultiReader
//------------------------------------------------------------------------------

QString CsvMultiReader::read()
{
    QFile f(fileName);
    if (!f.exists())
        return qApp->tr("File '%1' not found").arg(fileName);

    bool ok = f.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!ok)
        return qApp->tr("Failed to read file '%1': %2").arg(fileName, f.errorString());

    QTextStream stream(&f);
    LineSplitter lineSplitter(valueSeparators);
    ValueParser valueParser(decimalPoint);
    int linesSkipped = 0;
    while (true)
    {
        QString line;
        if (!stream.readLineInto(&line)) break;
        if (linesSkipped++ < skipFirstLines) continue;
        if (line.isEmpty()) continue;

        lineSplitter.split(line);
        int colCount = lineSplitter.parts.size();

        for (GraphItem& item : graphItems)
        {
            if (item.columnX < 1 || item.columnX > colCount) continue;
            if (item.columnY < 1 || item.columnY > colCount) continue;
            valueParser.parse(lineSplitter.parts.at(item.columnX-1));
            if (!valueParser.ok) continue;
            double x = valueParser.value;
            valueParser.parse(lineSplitter.parts.at(item.columnY-1));
            if (!valueParser.ok) continue;
            double y = valueParser.value;
            item.xs.append(x);
            item.ys.append(y);
        }
    }
    return QString();
}

//------------------------------------------------------------------------------
//                                  CsvSingleReader
//------------------------------------------------------------------------------

QString CsvSingleReader::read()
{
    QFile f(fileName);
    if (!f.exists())
        return qApp->tr("File '%1' not found").arg(fileName);

    bool ok = f.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!ok)
        return qApp->tr("Failed to read file '%1': %2").arg(fileName, f.errorString());

    QTextStream stream(&f);
    LineSplitter lineSplitter(valueSeparators);
    ValueParser valueParser(decimalPoint);
    int linesSkipped = 0;
    while (true)
    {
        QString line;
        if (!stream.readLineInto(&line)) break;
        if (linesSkipped++ < skipFirstLines) continue;
        if (line.isEmpty()) continue;

        lineSplitter.split(line);
        int colCount = lineSplitter.parts.size();

        if (columnX < 1 || columnX > colCount) continue;
        if (columnY < 1 || columnY > colCount) continue;
        valueParser.parse(lineSplitter.parts.at(columnX-1));
        if (!valueParser.ok) continue;
        double x = valueParser.value;
        valueParser.parse(lineSplitter.parts.at(columnY-1));
        if (!valueParser.ok) continue;
        double y = valueParser.value;
        xs.append(x);
        ys.append(y);
    }
    return QString();
}

//------------------------------------------------------------------------------
//                                TextReader
//------------------------------------------------------------------------------

QString TextReader::readFromFile()
{
    QFile f(fileName);
    if (!f.exists())
        return qApp->tr("File '%1' not found").arg(fileName);

    bool ok = f.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!ok)
        return qApp->tr("Failed to read file '%1': %2").arg(fileName, f.errorString());

    text = f.readAll();
    return QString();
}

QString TextReader::read()
{
    if (!fileName.isEmpty())
    {
        QString res = readFromFile();
        if (!res.isEmpty())
            return res;
    }

    if (text.isEmpty())
        return qApp->tr("Processing text is empty.");

    QVector<QStringRef> lines = text.splitRef('\n', QString::SkipEmptyParts);
    if (lines.isEmpty())
        return qApp->tr("Processing text is empty.");

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
        return qApp->tr("Processing text contains too few lines.");

    bool ok, gotX, gotY;
    double value, x, y;
    QVector<double> onlyY;

    for (const QStringRef& line : lines)
    {
        if (line.isEmpty()) continue;

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
    }

    if (ys.size() < 2 && onlyY.size() < 2)
        return qApp->tr("Too few points for plotting.");

    if (onlyY.size() > ys.size())
    {
        // treat data as single column
        ys = onlyY;
        xs.resize(ys.size());
        for (int i = 0; i < xs.size(); i++)
            xs[i] = i;
    }

    Q_ASSERT(xs.size() == ys.size());
    return QString();
}
