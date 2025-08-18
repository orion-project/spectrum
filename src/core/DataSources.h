#ifndef DATA_SOURCES_H
#define DATA_SOURCES_H

#include "BaseTypes.h"

class QJsonObject;

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

    /// Creates a title for graph line displayed in legend
    virtual QString makeTitle() const = 0;

    /// Some ident displayed to user to give a clue from what the graph has been built
    virtual QString displayStr() const { return QString(); }

    virtual QString canRefresh() const { return QString(); }
    virtual ConfigResult configure() { return ConfigResult(false); }
    
    virtual void save(QJsonObject &obj) const = 0;
    virtual void load(const QJsonObject &obj) = 0;
    
    const GraphPoints& data() { return _data; }
protected:
    GraphPoints _data;
};


class TextFileDataSource : public DataSource
{
public:
    TextFileDataSource() {}
    TextFileDataSource(QString fileName);
    ConfigResult configure() override;
    GraphResult read() override;
    QString makeTitle() const override;
    QString displayStr() const override { return _fileName; }
    void save(QJsonObject &obj) const override;
    void load(const QJsonObject &obj) override;
    static QString type() { return QStringLiteral("TextFile"); }
private:
    QString _fileName;
};


class CsvFileDataSource : public DataSource
{
public:
    ConfigResult configure() override;
    GraphResult read() override;
    QString makeTitle() const override;
    QString displayStr() const override;
    void save(QJsonObject &obj) const override;
    void load(const QJsonObject &obj) override;
    static QString type() { return QStringLiteral("CsvFile"); }
private:
    QString _fileName;
    CsvGraphParams _params;
    friend class CsvConfigDialog;
};


class RandomSampleDataSource : public DataSource
{
public:
    RandomSampleDataSource();
    RandomSampleDataSource(const RandomSampleParams& params);
    GraphResult read() override;
    QString makeTitle() const override;
    QString canRefresh() const override;
    QString displayStr() const override { return "Random sample"; }
    void save(QJsonObject &obj) const override;
    void load(const QJsonObject &obj) override;
    static QString type() { return QStringLiteral("RandomSample"); }
private:
    int _index;
    RandomSampleParams _params;
};


class ClipboardDataSource : public DataSource
{
public:
    ClipboardDataSource();
    GraphResult read() override;
    QString makeTitle() const override;
    QString canRefresh() const override;
    QString displayStr() const override { return "Clipboard"; }
    void save(QJsonObject &obj) const override;
    void load(const QJsonObject &obj) override;
    static QString type() { return QStringLiteral("Clipboard"); }
private:
    int _index;
};


class ClipboardCsvDataSource : public DataSource
{
public:
    GraphResult read() override;
    QString makeTitle() const override;
    QString canRefresh() const override;
    QString displayStr() const override { return "Clipboard"; }
    void save(QJsonObject &obj) const override;
    void load(const QJsonObject &obj) override;
    static QString type() { return QStringLiteral("ClipboardCsv"); }
private:
    CsvGraphParams _params;
    friend class CsvConfigDialog;
};

DataSource* makeDataSource(const QString &type);

#endif // DATA_SOURCES_H
