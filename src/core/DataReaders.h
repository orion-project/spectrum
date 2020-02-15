#ifndef DATA_READERS_H
#define DATA_READERS_H

#include "BaseTypes.h"

#include <QLocale>

struct LineSplitter
{
    LineSplitter(const QString& separators) : separators(separators) {}

    bool empty = true;
    QChar separator;
    QString separators;
    QVector<QStringRef> parts;
    QString::SplitBehavior splitBehavior;

    void detect(const QString& line);
    void split(const QString& line);
};

struct ValueParser
{
    ValueParser(bool decimalPoint)
    {
        locale = QLocale(decimalPoint ? QLocale::C : QLocale::Russian);
    }

    void parse(const QStringRef& s)
    {
        value = locale.toDouble(s, &ok);
    }

    bool ok;
    double value;
    QLocale locale;
};

struct ValueAutoParser
{
    void parse(const QStringRef& s);

    bool ok;
    double value;
    bool decimalPoint = true;
    QLocale localePoint = QLocale(QLocale::C);
    QLocale localeComma = QLocale(QLocale::Russian);
};

struct CsvMultiReader
{
    QString fileName;
    QString text;
    QString valueSeparators;
    bool decimalPoint;
    int skipFirstLines;

    struct GraphItem
    {
        QString title;
        int columnX, columnY;
        QVector<double> xs, ys;
    };
    QVector<GraphItem> graphItems;

    QString read();
    void read(QTextStream &stream);
    CsvGraphParams makeParams(const GraphItem& item) const;
};

struct CsvSingleReader
{
    QString fileName;
    CsvGraphParams params;
    QVector<double> xs, ys;

    QString read();
};

struct TextReader
{
    QString fileName;
    QString text;
    QVector<double> xs, ys;

    QString readFromFile();
    QString read();
};

#endif // DATA_READERS_H
