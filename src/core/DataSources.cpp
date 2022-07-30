#include "DataSources.h"

#include "DataReaders.h"
#include "../CustomPrefs.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>

static int __randomSampleIndex = 0;
static int __clipboardCallCount = 0;

//------------------------------------------------------------------------------
//                                 DataSource
//------------------------------------------------------------------------------

DataSource::~DataSource()
{
}

//------------------------------------------------------------------------------
//                             TextFileDataSource
//------------------------------------------------------------------------------

TextFileDataSource::TextFileDataSource(QString fileName): _fileName(fileName) {}

// Open another (or the same, if you wish) source file
bool TextFileDataSource::configure()
{
    Q_ASSERT(!_fileName.isEmpty());

    QFileDialog dlg(qApp->activeWindow());
    dlg.selectFile(_fileName);

    if (dlg.exec() != QDialog::Accepted)
        return false;

    auto files = dlg.selectedFiles();
    if (files.isEmpty()) return false;

    _fileName = files.first();

    auto root = CustomDataHelpers::loadDataSourceStates();
    auto state = root["file"].toObject();
    state["dir"] = dlg.directory().path();
    root["file"] = state;
    CustomDataHelpers::saveDataSourceStates(root);

    return true;
}

GraphResult TextFileDataSource::getData()
{
    TextReader reader;
    reader.fileName = _fileName;
    QString res = reader.read();
    if (!res.isEmpty())
        return GraphResult::fail(res);

    _initialData = {reader.xs, reader.ys};
    return GraphResult::ok(_initialData);
}

QString TextFileDataSource::makeTitle() const
{
    return QFileInfo(_fileName).fileName();
}

//------------------------------------------------------------------------------
//                             CsvFileMultiDataSource
//------------------------------------------------------------------------------

CsvFileDataSource::CsvFileDataSource(QString fileName): _fileName(fileName) {}

GraphResult CsvFileDataSource::getData()
{
    CsvSingleReader reader;
    reader.fileName = _fileName;
    reader.params = _params;
    QString res = reader.read();
    if (!res.isEmpty())
        return GraphResult::fail(res);

    _initialData = {reader.xs, reader.ys};
    return GraphResult::ok(_initialData);
}

QString CsvFileDataSource::makeTitle() const
{
    return _params.title;
}

//------------------------------------------------------------------------------
//                             RandomSampleDataSource
//------------------------------------------------------------------------------

RandomSampleDataSource::RandomSampleDataSource()
{
    _index = ++__randomSampleIndex;
}

GraphResult RandomSampleDataSource::getData()
{
    const double H = 25;
    const int count = 100;

    QVector<double> xs(count);
    QVector<double> ys(count);

    double y = (qrand()%100)*H*0.01;
    for (int i = 0; i < count; i++)
    {
        y = qAbs(y + (qrand()%100)*H*0.01 - H*0.5);

        xs[i] = i;
        ys[i] = y;
    }

    _initialData = {xs, ys};
    return GraphResult::ok(_initialData);
}

QString RandomSampleDataSource::canRefresh() const
{
    return qApp->tr("Refreshing of random graph data is not supported");
}

QString RandomSampleDataSource::makeTitle() const
{
     return QString("random-sample (%1)").arg(_index);
}

//------------------------------------------------------------------------------
//                             ClipboardDataSource
//------------------------------------------------------------------------------

ClipboardDataSource::ClipboardDataSource()
{
    if (qApp->clipboard()->mimeData()->hasText())
        _index = ++__clipboardCallCount;
}

GraphResult ClipboardDataSource::getData()
{
    QString text = qApp->clipboard()->text();
    if (text.isEmpty())
        return GraphResult::fail(qApp->tr("Clipboard does not contain suitable data"));

    TextReader reader;
    reader.text = text;
    QString res = reader.read();
    if (!res.isEmpty())
        return GraphResult::fail(res);

    _initialData = {reader.xs, reader.ys};
    return GraphResult::ok(_initialData);
}

QString ClipboardDataSource::canRefresh() const
{
    return qApp->tr("Refreshing of graph data from clipboard is not supported");
}

QString ClipboardDataSource::makeTitle() const
{
     return QString("clipboard (%1)").arg(_index);
}

//------------------------------------------------------------------------------
//                             ClipboardCsvDataSource
//------------------------------------------------------------------------------

GraphResult ClipboardCsvDataSource::getData()
{
    return GraphResult::fail("Getting data from Clipboard as CSV must be done via CsvConfigDialog::openClipboard()");
}

QString ClipboardCsvDataSource::canRefresh() const
{
    return qApp->tr("Refreshing of graph data from clipboard is not supported");
}

QString ClipboardCsvDataSource::makeTitle() const
{
     return _params.title;
}
