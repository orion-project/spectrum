#ifndef DATA_SOURCES_H
#define DATA_SOURCES_H

#include "BaseTypes.h"

class DataSource
{
public:
    virtual ~DataSource();
    virtual GraphResult getData() const = 0;
    virtual QString makeTitle() const = 0;
    virtual QString canRefresh() const { return QString(); }
};


class TextFileDataSource : public DataSource
{
public:
    TextFileDataSource(const QString& fileName);
    GraphResult getData() const override;
    QString makeTitle() const override;
private:
    QString _fileName;
};


class RandomSampleDataSource : public DataSource
{
public:
    RandomSampleDataSource();
    GraphResult getData() const override;
    QString makeTitle() const override;
    QString canRefresh() const override;
private:
    int _index;
};


class ClipboardDataSource : public DataSource
{
public:
    ClipboardDataSource();
    GraphResult getData() const override;
    QString makeTitle() const override;
    QString canRefresh() const override;
private:
    int _index;
};

#endif // DATA_SOURCES_H
