#ifndef DATA_SOURCES_H
#define DATA_SOURCES_H

#include "BaseTypes.h"

class DataSource
{
public:
    virtual ~DataSource();
    virtual GraphResult getData() = 0;
    virtual QString makeTitle() const = 0;
    virtual QString canRefresh() const { return QString(); }
    virtual bool configure() { return true; }
    const GraphPoints& initialData() { return _initialData; }
protected:
    GraphPoints _initialData;
};


class TextFileDataSource : public DataSource
{
public:
    TextFileDataSource();
    GraphResult getData() override;
    QString makeTitle() const override;
    bool configure() override;
private:
    QString _fileName;
};


class CsvFileDataSource : public DataSource
{
public:
    GraphResult getData() override;
    QString makeTitle() const override;
    bool configure() override;
private:
    QString _fileName, _title;
    QString _valueSeparators;
    bool _decimalPoint;
    int _columnX, _columnY;
    int _skipFirstLines;
    friend class CsvConfigDialog;
};

class RandomSampleDataSource : public DataSource
{
public:
    RandomSampleDataSource();
    GraphResult getData() override;
    QString makeTitle() const override;
    QString canRefresh() const override;
private:
    int _index;
};


class ClipboardDataSource : public DataSource
{
public:
    ClipboardDataSource();
    GraphResult getData() override;
    QString makeTitle() const override;
    QString canRefresh() const override;
private:
    int _index;
};

#endif // DATA_SOURCES_H
