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
    
    virtual QString type() const = 0;

    /// Creates a title for graph line displayed in legend
    virtual QString makeTitle() const = 0;

    /// Some ident displayed to user to give a clue from what the graph has been built
    virtual QString displayStr() const { return QString(); }

    virtual QString canRefresh() const { return QString(); }
    virtual ConfigResult configure() { return ConfigResult(false); }
    
    virtual void save(QJsonObject &obj) const = 0;
    virtual void load(const QJsonObject &obj) = 0;
    
    const GraphPoints& data() { return _data; }
    virtual void copyParams(DataSource *other) = 0;
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
    QString type() const override { return _type_(); }
    static QString _type_() { return QStringLiteral("TextFile"); }
    void copyParams(DataSource *other) override;
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
    QString type() const override { return _type_(); }
    static QString _type_() { return QStringLiteral("CsvFile"); }
    void copyParams(DataSource *other) override;
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
    QString type() const override { return _type_(); }
    static QString _type_() { return QStringLiteral("RandomSample"); }
    void copyParams(DataSource *other) override;
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
    QString type() const override { return _type_(); }
    static QString _type_() { return QStringLiteral("Clipboard"); }
    void copyParams(DataSource *other) override {}
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
    QString type() const override { return _type_(); }
    static QString _type_() { return QStringLiteral("ClipboardCsv"); }
    void copyParams(DataSource *other) override;
private:
    CsvGraphParams _params;
    friend class CsvConfigDialog;
};


class FormulaDataSource : public DataSource
{
public:
    FormulaDataSource();
    FormulaDataSource(const QString& code);
    ConfigResult configure() override;
    GraphResult read() override;
    QString makeTitle() const override;
    QString displayStr() const override { return QStringLiteral("Formula"); }
    void save(QJsonObject &obj) const override;
    void load(const QJsonObject &obj) override;
    QString type() const override { return _type_(); }
    static QString _type_() { return QStringLiteral("Formula"); }
    void copyParams(DataSource *other) override;
    QString code() const { return _code; }
    
    static GraphResult exec(const QString &code);
    
private:
    int _index;
    QString _code;
    bool _dataReady = false;
};

DataSource* makeDataSource(const QString &type);

#endif // DATA_SOURCES_H
