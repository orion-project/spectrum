#include "DataSources.h"

#include "CustomPrefs.h"
#include "core/DataReaders.h"

#include "helpers/OriTools.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>

//------------------------------------------------------------------------------
//                                 DataSource
//------------------------------------------------------------------------------

DataSource::~DataSource()
{
}

//------------------------------------------------------------------------------
//                                 Helpers
//------------------------------------------------------------------------------

static QString reselectFile(QString oldFile)
{
    Q_ASSERT(!oldFile.isEmpty());

    QFileDialog dlg(qApp->activeWindow());
    dlg.selectFile(oldFile);

    if (dlg.exec() != QDialog::Accepted)
        return QString();

    auto files = dlg.selectedFiles();
    if (files.isEmpty())
        return QString();

    auto root = CustomDataHelpers::loadDataSourceStates();
    auto state = root["file"].toObject();
    state["dir"] = dlg.directory().path();
    root["file"] = state;
    CustomDataHelpers::saveDataSourceStates(root);

    return files.first();
}

//------------------------------------------------------------------------------
//                             TextFileDataSource
//------------------------------------------------------------------------------

TextFileDataSource::TextFileDataSource(QString fileName): _fileName(fileName) {}

// Open another (or the same, if you wish) source file
DataSource::ConfigResult TextFileDataSource::configure()
{
    QString fileName = reselectFile(_fileName);
    if (fileName.isEmpty())
        return ConfigResult(false);

    _fileName = fileName;
    return ConfigResult(true);
}

GraphResult TextFileDataSource::read()
{
    TextReader reader;
    reader.fileName = _fileName;
    QString res = reader.read();
    if (!res.isEmpty())
        return GraphResult::fail(res);

    _data = {reader.xs, reader.ys};
    return GraphResult::ok(_data);
}

QString TextFileDataSource::makeTitle() const
{
    return QFileInfo(_fileName).fileName();
}

//------------------------------------------------------------------------------
//                             CsvFileDataSource
//------------------------------------------------------------------------------

// Open another (or the same, if you wish) source file
DataSource::ConfigResult CsvFileDataSource::configure()
{
    QString fileName = reselectFile(_fileName);
    if (fileName.isEmpty())
        return ConfigResult(false);

    _fileName = fileName;
    return ConfigResult(true);
}

GraphResult CsvFileDataSource::read()
{
    CsvSingleReader reader;
    reader.fileName = _fileName;
    reader.params = _params;
    QString res = reader.read();
    if (!res.isEmpty())
        return GraphResult::fail(res);

    _data = {reader.xs, reader.ys};
    return GraphResult::ok(_data);
}

QString CsvFileDataSource::makeTitle() const
{
    QString title = _params.title;
    if (!_fileName.isEmpty())
    {
        QString fileName = QFileInfo(_fileName).fileName();
        static QRegularExpression reFilename("^.*(\\{ds\\}).*$");
        if (auto m = reFilename.match(title); m.hasMatch())
        {
            int start = m.capturedStart(1);
            int len = m.capturedLength(1);
            if (start > -1 && len > 0)
                title.replace(start, len, fileName);
        }
    }
    return title;
}

QString CsvFileDataSource::displayStr() const
{
    if (_params.columnX >= 0)
        return QStringLiteral("%1 [%2;%3]").arg(_fileName).arg(_params.columnX).arg(_params.columnY);
    return QStringLiteral("%1 [%2]").arg(_fileName).arg(_params.columnY);
}

//------------------------------------------------------------------------------
//                             RandomSampleDataSource
//------------------------------------------------------------------------------

RandomSampleDataSource::RandomSampleDataSource(const RandomSampleParams& params)
{
    static int randomSampleIndex = 0;
    _index = ++randomSampleIndex;
    _params = params;
}

GraphResult RandomSampleDataSource::read()
{
    QVector<double> xs = _params.rangeX.calcValues();
    QVector<double> ys(xs.size());
    const double Y = _params.rangeY.max - _params.rangeY.min;
    const double H = Y / 4.0;
    const double max = double(std::numeric_limits<quint32>::max());
    double y = double(Ori::Tools::rand())/max * H;
    for (int i = 0; i < xs.size(); i++)
    {
        y = qAbs(y + double(Ori::Tools::rand())/max*H - H*0.5);
        if (y > Y)
            y = Y - double(Ori::Tools::rand())/max*H;
        ys[i] = y + _params.rangeY.min;
    }
    _data = {xs, ys};
    return GraphResult::ok(_data);
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
    static int clipboardCallCount = 0;
    if (qApp->clipboard()->mimeData()->hasText())
        _index = ++clipboardCallCount;
}

GraphResult ClipboardDataSource::read()
{
    QString text = qApp->clipboard()->text();
    if (text.isEmpty())
        return GraphResult::fail(qApp->tr("Clipboard does not contain appropriate data"));

    TextReader reader;
    reader.text = text;
    QString res = reader.read();
    if (!res.isEmpty())
        return GraphResult::fail(res);

    _data = {reader.xs, reader.ys};
    return GraphResult::ok(_data);
}

QString ClipboardDataSource::canRefresh() const
{
    return qApp->tr("Rereading of clipboard not available");
}

QString ClipboardDataSource::makeTitle() const
{
     return QString("clipboard (%1)").arg(_index);
}

//------------------------------------------------------------------------------
//                             ClipboardCsvDataSource
//------------------------------------------------------------------------------

GraphResult ClipboardCsvDataSource::read()
{
    return GraphResult::fail("Getting data from Clipboard as CSV must be done via CsvConfigDialog::openClipboard()");
}

QString ClipboardCsvDataSource::canRefresh() const
{
    return qApp->tr("Rereading of clipboard not available");
}

QString ClipboardCsvDataSource::makeTitle() const
{
     return _params.title;
}
