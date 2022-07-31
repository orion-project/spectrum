#ifndef DATA_SOURCES_H
#define DATA_SOURCES_H

#include "BaseTypes.h"

class DataSource
{
public:
    struct ConfigResult
    {
        bool ok = false;
        QString error;

        ConfigResult(bool ok): ok(ok) {}
    };

    virtual ~DataSource();
    virtual GraphResult read() = 0;
    virtual QString makeTitle() const = 0;
    virtual QString canRefresh() const { return QString(); }
    virtual ConfigResult configure() { return ConfigResult(false); }
    const GraphPoints& data() { return _data; }
protected:
    GraphPoints _data;
};


class TextFileDataSource : public DataSource
{
public:
    TextFileDataSource(QString fileName);
    ConfigResult configure() override;
    GraphResult read() override;
    QString makeTitle() const override;
private:
    QString _fileName;
};


class CsvFileDataSource : public DataSource
{
public:
    ConfigResult configure() override;
    GraphResult read() override;
    QString makeTitle() const override;
private:
    QString _fileName;
    CsvGraphParams _params;
    friend class CsvConfigDialog;
};

class RandomSampleDataSource : public DataSource
{
public:
    RandomSampleDataSource();
    GraphResult read() override;
    QString makeTitle() const override;
    QString canRefresh() const override;
private:
    int _index;
};


class ClipboardDataSource : public DataSource
{
public:
    ClipboardDataSource();
    GraphResult read() override;
    QString makeTitle() const override;
    QString canRefresh() const override;
private:
    int _index;
};


class ClipboardCsvDataSource : public DataSource
{
public:
    GraphResult read() override;
    QString makeTitle() const override;
    QString canRefresh() const override;
private:
    CsvGraphParams _params;
    friend class CsvConfigDialog;
};

#endif // DATA_SOURCES_H
