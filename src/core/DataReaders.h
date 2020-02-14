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

struct CsvMultiReader
{
    QString fileName;
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
};

struct CsvSingleReader
{
    QString fileName;
    QString valueSeparators;
    bool decimalPoint;
    int skipFirstLines;
    int columnX, columnY;
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
