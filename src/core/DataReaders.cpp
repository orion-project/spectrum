#include "DataReaders.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>

static const QVector<QPair<QChar, Qt::SplitBehavior>>& defaultValueSeparators()
{
    // a line like "1,,3" should issue 3 columns
    // a line like "1    3" should issue only 2 columns
    static QVector<QPair<QChar, Qt::SplitBehavior>> def = {
        qMakePair(',', Qt::KeepEmptyParts),
        qMakePair(';', Qt::KeepEmptyParts),
        qMakePair(' ', Qt::SkipEmptyParts),
        qMakePair('\t', Qt::SkipEmptyParts),
    };
    return def;
}

//------------------------------------------------------------------------------
//                               LineSplitter
//------------------------------------------------------------------------------

LineSplitter::LineSplitter(const QString& seps)
{
    for (const auto ch : seps) {
        separators.append(ch);
    }
}

void LineSplitter::splitAuto(QStringView line)
{
    parts.clear();
    for (const auto &ch : line) {
        for (const auto &s : defaultValueSeparators()) {
            if (ch == s.first) {
                parts = QStringView(line).split(s.first, s.second);
                qDebug() << "sep" << s.first << "parts" << parts;
                if (parts.size() > 1)
                {
                    return;
                }
            }
        }
    }
}

void LineSplitter::split(QStringView line)
{
    if (separators.isEmpty()) {
        splitAuto(line);
        return;
    }
    // TODO
}

//------------------------------------------------------------------------------
//                                 ValueParser
//------------------------------------------------------------------------------

void ValueParser::parse(const QStringView &s)
{
    QStringView s1;
    if (stripNonDigits)
    {
        int len = s.length();
        int start = 0;
        while (start < len)
        {
            QChar ch = s.at(start);
            if (ch.isDigit() || ch == '+' || ch == '-')
                break;
            start++;
        }
        int stop = len - 1;
        while (stop > start)
        {
            if ( s.at(stop).isDigit())
                break;
            stop--;
        }
        s1 = s.mid(start, stop-start+1);
    }
    else s1 = s;
    value = locale.toDouble(s1, &ok);
}

//------------------------------------------------------------------------------
//                                 ValueAutoParser
//------------------------------------------------------------------------------

void ValueAutoParser::parse(const QStringView& s)
{
    ok = false;
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

    auto lines = QStringView(text).split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty())
        return qApp->tr("Processing text is empty.");

    // It may be line of numbers, we can plot it spliting by a separator
    // 0.403922 0.419608 0.443137 0.458824 0.458824 0.466667 0.482353...
    if (lines.size() == 1)
    {
        for (const auto& s : defaultValueSeparators())
        {
            lines = QStringView(text).split(s.first, s.second);
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

    for (const QStringView& line : qAsConst(lines))
    {
        if (line.isEmpty()) continue;

        QList<QStringView> parts;
        for (const auto& s : defaultValueSeparators())
        {
            parts = line.split(s.first, s.second);
            if (parts.size() > 1) break;
        }

        gotX = gotY = false;
        for (const QStringView& part : qAsConst(parts))
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
