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

//------------------------------------------------------------------------------
//                                 ValueAutoParser
//------------------------------------------------------------------------------

void ValueAutoParser::parse(const QStringRef& s)
{
    if (decimalPoint)
    {
        value = localePoint.toDouble(s, &ok);
        if (ok) return;
        value = localeComma.toDouble(s, &ok);
        if (ok) decimalPoint = false;
    }
    else
    {
        value = localeComma.toDouble(s, &ok);
        if (ok) return;
        value = localePoint.toDouble(s, &ok);
        if (ok) decimalPoint = true;
    }
}

//------------------------------------------------------------------------------
//                                  CsvMultiReader
//------------------------------------------------------------------------------

QString CsvMultiReader::read()
{
    Q_ASSERT(!fileName.isEmpty() || !text.isEmpty());

    if (!fileName.isEmpty())
    {
        QFile f(fileName);
        if (!f.exists())
            return qApp->tr("File '%1' not found").arg(fileName);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            return qApp->tr("Failed to read file '%1': %2").arg(fileName, f.errorString());

        QTextStream stream(&f);
        read(stream);
        return QString();
    }
    else
    {
        QTextStream stream(&text);
        read(stream);
        return QString();
    }
}

void CsvMultiReader::read(QTextStream& stream)
{
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
}

CsvGraphParams CsvMultiReader::makeParams(const GraphItem &item) const
{
    CsvGraphParams p;
    p.title = item.title;
    p.columnX = item.columnX;
    p.columnY = item.columnY;
    p.valueSeparators = valueSeparators;
    p.skipFirstLines = skipFirstLines;
    p.decimalPoint = decimalPoint;
    return p;
}

//------------------------------------------------------------------------------
//                                  CsvSingleReader
//------------------------------------------------------------------------------

QString CsvSingleReader::read()
{
    QFile f(fileName);
    if (!f.exists())
        return qApp->tr("File '%1' not found").arg(fileName);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return qApp->tr("Failed to read file '%1': %2").arg(fileName, f.errorString());

    QTextStream stream(&f);
    LineSplitter lineSplitter(params.valueSeparators);
    ValueParser valueParser(params.decimalPoint);
    int linesSkipped = 0;
    while (true)
    {
        QString line;
        if (!stream.readLineInto(&line)) break;
        if (linesSkipped++ < params.skipFirstLines) continue;
        if (line.isEmpty()) continue;

        lineSplitter.split(line);
        int colCount = lineSplitter.parts.size();

        if (params.columnX < 1 || params.columnX > colCount) continue;
        if (params.columnY < 1 || params.columnY > colCount) continue;
        valueParser.parse(lineSplitter.parts.at(params.columnX-1));
        if (!valueParser.ok) continue;
        double x = valueParser.value;
        valueParser.parse(lineSplitter.parts.at(params.columnY-1));
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
        return qApp->tr("Processing text contains too few lines.");

    bool gotX, gotY;
    double x, y;
    QVector<double> onlyY;
    ValueAutoParser valueParser;

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
            valueParser.parse(part);
            if (!valueParser.ok)
                continue;
            if (!gotX)
            {
                x = valueParser.value;
                gotX = true;
            }
            else
            {
                y = valueParser.value;
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
