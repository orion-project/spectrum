#include "CsvConfigDialog.h"

#include "CustomPrefs.h"
#include "core/DataReaders.h"
#include "core/DataSources.h"

#include "helpers/OriDialogs.h"
#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QTableView>
#include <QTextStream>
#include <QSpinBox>

#define DEFAULT_COL_X 1
#define DEFAULT_COL_Y 2
#define DEFAULT_DLG_W 750
#define DEFAULT_DLG_H 550

using namespace Ori::Layouts;

struct CsvOpenerState
{
    CsvOpenerState()
    {
        root = CustomDataHelpers::loadCustomData("datasources");
        file = root["file"].toObject();
        csv = root["csv"].toObject();
    }

    void restore(CsvConfigDialog& paramsEditor)
    {
        paramsEditor._valueSeparator->setText(csv["value_separator"].toString(",;"));
        paramsEditor._skipFirstLines->setValue(csv["skip_first_lines"].toInt(0));
        paramsEditor._previewLinesCount->setValue(csv["preview_lines_count"].toInt(15));
        paramsEditor._decSepPoint->setChecked(csv["dep_sep_point"].toBool(true));
        paramsEditor._decSepComma->setChecked(!csv["dep_sep_point"].toBool(true));

        auto jsonGraphs = csv["graphs"].toArray();
        for (auto it = jsonGraphs.begin(); it != jsonGraphs.end(); it++)
        {
            auto graphItem = (*it).toObject();
            int colX = graphItem["col_x"].toInt(0);
            int colY = graphItem["col_y"].toInt(0);
            if (colX > 0 && colY > 0)
                paramsEditor.addGraphItem(colX, colY);
        }
    }

    void store(CsvConfigDialog& paramsEditor)
    {
        QJsonArray jsonGraphs;
        for (auto item : paramsEditor._graphsItems)
            jsonGraphs.append(QJsonObject({{"col_x", item.colX->value()},
                                           {"col_y", item.colY->value()}}));
        csv["graphs"] = jsonGraphs;
        csv["value_separator"] = paramsEditor._valueSeparator->text().trimmed();
        csv["dep_sep_point"] = paramsEditor._decSepPoint->isChecked();
        csv["skip_first_lines"] = paramsEditor._skipFirstLines->value();
        csv["preview_lines_count"] = paramsEditor._previewLinesCount->value();
    }

    void save()
    {
        root["file"] = file;
        root["csv"] = csv;
        CustomDataHelpers::saveCustomData(root, "datasources");
    }

    QJsonObject root, file, csv;
};

struct CsvOpenFileDlg
{
    QString fileName, baseName;

    bool open(CsvOpenerState& state)
    {
        QFileDialog fileDlg(qApp->activeWindow());

        if (fileName.isEmpty())
        {
            auto dir = state.file["dir"].toString();
            if (!dir.isEmpty())
                fileDlg.setDirectory(dir);
        }
        else fileDlg.selectFile(fileName);

        if (fileDlg.exec() != QDialog::Accepted)
            return false;

        auto files = fileDlg.selectedFiles();
        if (files.isEmpty())
            return false;

        fileName = files.first();
        if (fileName.isEmpty())
            return false;

        baseName = makeBaseName(fileName);

        state.file["dir"] = fileDlg.directory().path();
        return true;
    }

    QString makeBaseName(const QString& fileName)
    {
        return QFileInfo(fileName).fileName();
    }
};

CsvOpenResult CsvConfigDialog::openFile()
{
    CsvOpenerState state;

    CsvOpenFileDlg fileDlg;
    if (!fileDlg.open(state)) return CsvOpenResult::ok({});

    CsvConfigDialog paramsEditor;
    paramsEditor._fileName = fileDlg.fileName;
    paramsEditor._graphBaseName = fileDlg.baseName;
    state.restore(paramsEditor); // do after setting `_graphBaseName`

    if (!paramsEditor.showDialog(fileDlg.fileName, state))
        return CsvOpenResult::ok({});

    state.store(paramsEditor);
    state.save();

    CsvMultiReader csvReader;
    paramsEditor.initReader(csvReader);
    QString res = csvReader.read();
    if (!res.isEmpty()) return CsvOpenResult::fail(res);

    QVector<DataSource*> dataSources;
    for (const CsvMultiReader::GraphItem& item : csvReader.graphItems)
    {
        Q_ASSERT(item.xs.size() == item.ys.size());
        if (item.xs.isEmpty()) continue;
        auto dataSource = new CsvFileDataSource;
        dataSource->_fileName = csvReader.fileName;
        dataSource->_params = csvReader.makeParams(item);
        dataSource->_initialData.xs = item.xs;
        dataSource->_initialData.ys = item.ys;
        dataSources << dataSource;
    }
    return CsvOpenResult::ok(dataSources);
}

CsvOpenResult CsvConfigDialog::openClipboard()
{
    QString text = qApp->clipboard()->text();
    if (text.isEmpty())
        return CsvOpenResult::fail(qApp->tr("Clipboard does not contain suitable data"));

    CsvOpenerState state;

    CsvConfigDialog paramsEditor;
    paramsEditor._text = text;
    paramsEditor._graphBaseName = "clipboard";
    state.restore(paramsEditor); // do after setting `_graphBaseName`

    if (!paramsEditor.showDialog(tr("Paste as CSV"), state))
        return CsvOpenResult::ok({});

    state.store(paramsEditor);
    state.save();

    CsvMultiReader csvReader;
    paramsEditor.initReader(csvReader);
    QString res = csvReader.read();
    if (!res.isEmpty())
        return CsvOpenResult::fail(res);

    QVector<DataSource*> dataSources;
    for (const CsvMultiReader::GraphItem& item : csvReader.graphItems)
    {
        Q_ASSERT(item.xs.size() == item.ys.size());
        if (item.xs.isEmpty()) continue;
        auto dataSource = new ClipboardCsvDataSource;
        dataSource->_params = csvReader.makeParams(item);
        dataSource->_initialData.xs = item.xs;
        dataSource->_initialData.ys = item.ys;
        dataSources << dataSource;
    }
    return CsvOpenResult::ok(dataSources);
}

bool CsvConfigDialog::openExisted(CsvFileDataSource* dataSource)
{
    CsvOpenerState state;

    CsvOpenFileDlg fileDlg;
    fileDlg.fileName = dataSource->_fileName;
    if (!fileDlg.open(state)) return false;

    CsvConfigDialog paramsEditor(true);
    paramsEditor._fileName = fileDlg.fileName;
    paramsEditor._graphBaseName = fileDlg.baseName;
    paramsEditor._valueSeparator->setText(dataSource->_params.valueSeparators);
    paramsEditor._skipFirstLines->setValue(dataSource->_params.skipFirstLines);
    paramsEditor._previewLinesCount->setValue(state.csv["preview_lines_count"].toInt(15));
    paramsEditor._decSepPoint->setChecked(dataSource->_params.decimalPoint);
    paramsEditor._decSepComma->setChecked(!dataSource->_params.decimalPoint);

    QString oldBaseName = fileDlg.makeBaseName(dataSource->_fileName);
    QString autoTitlePattern = QString("^%1\\s+\\[\\s*\\d+\\s*[,;]\\s*\\d+\\s*\\].*$").arg(oldBaseName);
    bool isAutoTitle = QRegularExpression(autoTitlePattern).match(dataSource->_params.title).hasMatch();
    QString oldGraphTitle = isAutoTitle ? QString() : dataSource->_params.title;
    paramsEditor.addGraphItem(dataSource->_params.columnX, dataSource->_params.columnY, oldGraphTitle);

    if (!paramsEditor.showDialog(fileDlg.fileName, state))
        return false;

    state.csv["preview_lines_count"] = paramsEditor._previewLinesCount->value();
    state.save();

    dataSource->_fileName = fileDlg.fileName;
    dataSource->_params.valueSeparators = paramsEditor._valueSeparator->text().trimmed();
    dataSource->_params.skipFirstLines = paramsEditor._skipFirstLines->value();
    dataSource->_params.decimalPoint = paramsEditor._decSepPoint->isChecked();
    auto item = paramsEditor._graphsItems.first();
    dataSource->_params.columnX = item.colX->value();
    dataSource->_params.columnY = item.colY->value();
    dataSource->_params.title = item.title->text().trimmed();
    return true;
}

namespace {
class ExpandingTextEdit : public QPlainTextEdit
{
public:
    QSize sizeHint() const override { return QSize(9999, 9999); }
};


class PreviewDataModel : public QAbstractItemModel
{
public:
    PreviewDataModel(QObject* parent): QAbstractItemModel(parent) {}
    QModelIndex index(int row, int col, const QModelIndex &parent) const override { Q_UNUSED(parent) return createIndex(row, col); }
    QModelIndex parent(const QModelIndex &child) const override { Q_UNUSED(child) return QModelIndex(); }
    int rowCount(const QModelIndex &parent) const override { Q_UNUSED(parent) return _values.size(); }
    int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent) return _colCount; }
    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role != Qt::DisplayRole) return QVariant();
        const QStringList& cols = _values.at(index.row());
        int col = index.column();
        if (col < 0 || col > cols.size()-1) return QVariant();
        return cols.at(col);
    }
    void setPreviewData(int colCount, const QVector<QStringList>& values)
    {
        beginResetModel();
        _colCount = colCount;
        _values = values;
        endResetModel();
    }
private:
    int _colCount;
    QVector<QStringList> _values;
};
} // namespace

CsvConfigDialog::CsvConfigDialog(bool editMode) : QWidget(), _editMode(editMode)
{
    _fileTextPreview = new ExpandingTextEdit;
    _fileTextPreview->setReadOnly(true);
    Ori::Gui::setFontMonospace(_fileTextPreview);

    _dataTablePreview = new QTableView;
    _dataTablePreview->setModel(new PreviewDataModel(this));
    Ori::Gui::adjustFont(_dataTablePreview);

    _buttonShowText = new QPushButton(tr("File text"));
    _buttonShowText->setCheckable(true);
    connect(_buttonShowText, &QPushButton::clicked, [this](){
        _buttonShowText->setChecked(true);
        _buttonShowData->setChecked(false);
        _tabs->setCurrentIndex(0);
    });

    _buttonShowData = new QPushButton(tr("Data table"));
    _buttonShowData->setCheckable(true);
    _buttonShowData->setChecked(true);
    connect(_buttonShowData, &QPushButton::clicked, [this](){
        _buttonShowText->setChecked(false);
        _buttonShowData->setChecked(true);
        _tabs->setCurrentIndex(1);
    });

    _tabs = new QStackedWidget;
    _tabs->addWidget(_fileTextPreview);
    _tabs->addWidget(_dataTablePreview);
    _tabs->setCurrentIndex(1);

    _valueSeparator = new QLineEdit;
    connect(_valueSeparator, &QLineEdit::textEdited, this, &CsvConfigDialog::updatePreviewData);

    _decSepPoint = new QRadioButton(tr("Point"));
    connect(_decSepPoint, &QRadioButton::clicked, this, &CsvConfigDialog::updatePreviewData);

    _decSepComma = new QRadioButton(tr("Comma"));
    connect(_decSepComma, &QRadioButton::clicked, this, &CsvConfigDialog::updatePreviewData);

    _skipFirstLines = new QSpinBox;
    connect(_skipFirstLines, QOverload<int>::of(&QSpinBox::valueChanged), this, &CsvConfigDialog::updateFullPreview);

    _previewLinesCount = new QSpinBox;
    connect(_previewLinesCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &CsvConfigDialog::updateFullPreview);

    auto optionsGroup = new QGroupBox(tr("Options"));
    auto optionsLayout = new QFormLayout(optionsGroup);
    optionsLayout->setHorizontalSpacing(12);
    optionsLayout->addRow(tr("Value separators"), _valueSeparator);
    optionsLayout->addRow(tr("Decimal separator"), LayoutH({_decSepPoint, _decSepComma}).setMargin(0).boxLayout());
    optionsLayout->addRow(tr("Skip first lines"), _skipFirstLines);
    optionsLayout->addRow(tr("Preview lines"), _previewLinesCount);

    auto graphsGroup = new QGroupBox(tr("Graphs"));
    _layoutGraphs = new QGridLayout;
    _layoutGraphs->setMargin(3);
    _layoutGraphs->setHorizontalSpacing(6);
    _layoutGraphs->setVerticalSpacing(3);
    LayoutV({_layoutGraphs, Stretch()}).setMargin(0).useFor(graphsGroup);

    QPushButton *buttonAddGraph = nullptr;
    if (!_editMode)
    {
        buttonAddGraph = new QPushButton(tr("+ Graph"));
        connect(buttonAddGraph, &QPushButton::clicked, [this](){ addGraphItem(DEFAULT_COL_X, DEFAULT_COL_Y); });
    }

    LayoutV({
        LayoutH({ optionsGroup, graphsGroup })
                .setStretchFactor(optionsGroup, 1)
                .setStretchFactor(graphsGroup, 100)
                .setMargin(0),
        LayoutH({ _buttonShowText, _buttonShowData, Stretch(), buttonAddGraph}).setMargin(0),
        _tabs
    }).setMargin(0).useFor(this);
}

bool CsvConfigDialog::showDialog(const QString &title, CsvOpenerState &state)
{
    updateFullPreview();

    auto validator = [this](){
        if (_graphsItems.isEmpty())
            return tr("There is nothing to plot!");
        return QString();
    };

    auto dlg = Ori::Dlg::Dialog(this, false)
            .withInitialSize({state.csv["window_width"].toInt(DEFAULT_DLG_W),
                              state.csv["window_height"].toInt(DEFAULT_DLG_H)})
            .withVerification(validator)
            .withTitle(title);
    if (!dlg.exec()) return false;
    auto size = dlg.size();
    state.csv["window_width"] = size.width();
    state.csv["window_height"] = size.height();
    return true;
}

void CsvConfigDialog::initReader(CsvMultiReader& reader)
{
    reader.text = _text;
    reader.fileName = _fileName;
    reader.decimalPoint = _decSepPoint->isChecked();
    reader.skipFirstLines = _skipFirstLines->value();
    reader.valueSeparators = _valueSeparator->text().trimmed();

    for (const CvsGraphItemView& item : _graphsItems)
    {
        CsvMultiReader::GraphItem it;
        it.columnX = item.colX->value();
        it.columnY = item.colY->value();
        it.title = item.title->text().trimmed();
        reader.graphItems.append(it);
    }
}

void CsvConfigDialog::updateFullPreview()
{
    Q_ASSERT(!_fileName.isEmpty() || !_text.isEmpty());
    updatePreviewText();
    updatePreviewData();
}

void CsvConfigDialog::updatePreviewText()
{
    _previewLines.clear();
    _fileTextPreview->clear();

    if (!_fileName.isEmpty())
    {
        QFile file(_fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            _fileTextPreview->setPlainText(qApp->tr("Failed to read the file: %2").arg(file.errorString()));
            return;
        }
        QTextStream stream(&file);
        updatePreviewText(stream);
    }
    else
    {
        QTextStream stream(&_text);
        updatePreviewText(stream);
    }
}

void CsvConfigDialog::updatePreviewText(QTextStream& stream)
{
    int linesToRead = _previewLinesCount->value();
    int linesToSkip = _skipFirstLines->value();
    int linesRead = 0;
    int linesSkipped = 0;
    while (linesRead++ < linesToRead)
    {
        QString line;
        if (!stream.readLineInto(&line)) break;
        if (linesSkipped++ < linesToSkip) continue;
        if (line.isEmpty()) continue;
        _previewLines << line;
    }
    _fileTextPreview->setPlainText(_previewLines.join('\n'));
}

void CsvConfigDialog::updatePreviewData()
{
    QVector<QStringList> rows;
    auto tableModel = dynamic_cast<PreviewDataModel*>(_dataTablePreview->model());
    if (_previewLines.isEmpty())
    {
        tableModel->setPreviewData(0, rows);
        return;
    }
    LineSplitter lineSplitter(_valueSeparator->text().trimmed());
    ValueParser valueParser(_decSepPoint->isChecked());
    int maxColCount = 0;
    for (auto line : _previewLines)
    {
        lineSplitter.split(line);
        QStringList rowValues;
        for (const QStringRef& part : lineSplitter.parts)
        {
            valueParser.parse(part);
            rowValues << (valueParser.ok ? part.toString() : QStringLiteral("NaN"));
        }
        if (rowValues.size() > maxColCount)
            maxColCount = rowValues.size();
        rows << rowValues;
    }
    tableModel->setPreviewData(maxColCount, rows);
    _dataTablePreview->resizeColumnsToContents();
    _dataTablePreview->resizeRowsToContents();
}

void CsvConfigDialog::addGraphItem(int colX, int colY, const QString& title)
{
    int row = _graphsItems.size();
    CvsGraphItemView item;
    item.labX = new QLabel("  X:");
    item.labY = new QLabel("  Y:");
    item.labTitle = new QLabel(tr("    Title:"));
    item.colX = new QSpinBox;
    item.colY = new QSpinBox;
    item.title = new QLineEdit;
    Ori::Gui::adjustFont(item.colX);
    Ori::Gui::adjustFont(item.colY);
    Ori::Gui::adjustFont(item.title);
    item.colX->setValue(colX);
    item.colY->setValue(colY);
    item.title->setText(title.isEmpty() ? QString("%1 [%2;%3]").arg(_graphBaseName).arg(colX).arg(colY) : title);

    auto updateGraphTitle = [item](){
        QString title = item.title->text();
        auto m = QRegularExpression("^.*\\[\\s*(\\d+)\\s*[,;]\\s*(\\d+)\\s*\\].*$").match(title);
        if (!m.hasMatch()) return;
        int startX = m.capturedStart(1);
        int lenX = m.capturedLength(1);
        int startY = m.capturedStart(2);
        int lenY = m.capturedLength(2);
        if (startX > -1 && lenX > 0 && startY > -1 && lenY > 0)
        {
            title.replace(startX, lenX, QString::number(item.colX->value()));
            title.replace(startY, lenY, QString::number(item.colY->value()));
            item.title->setText(title);
        }
    };
    connect(item.colX, QOverload<int>::of(&QSpinBox::valueChanged), updateGraphTitle);
    connect(item.colY, QOverload<int>::of(&QSpinBox::valueChanged), updateGraphTitle);

    if (!_editMode)
    {
        item.buttonDel = new QPushButton;
        item.buttonDel->setFlat(true);
        item.buttonDel->setIcon(QIcon(":/toolbar/delete"));
        item.buttonDel->setToolTip(tr("Remove graph"));
        item.buttonDel->setFixedWidth(24);
        connect(item.buttonDel, &QPushButton::clicked, [this, item](){
            if (!Ori::Dlg::yes(tr("Remove graph?"))) return;
            item.labX->deleteLater();
            item.labY->deleteLater();
            item.labTitle->deleteLater();
            item.colX->deleteLater();
            item.colY->deleteLater();
            item.title->deleteLater();
            item.buttonDel->deleteLater();
            for (int i = 0; i < _graphsItems.size(); i++)
                if (_graphsItems.at(i).colX == item.colX)
                {
                    _graphsItems.removeAt(i);
                    break;
                }
        });
    }

    int col = 0;
    _layoutGraphs->addWidget(item.labX, row, col++);
    _layoutGraphs->addWidget(item.colX, row, col++);
    _layoutGraphs->addWidget(item.labY, row, col++);
    _layoutGraphs->addWidget(item.colY, row, col++);
    _layoutGraphs->addWidget(item.labTitle, row, col++);
    _layoutGraphs->addWidget(item.title, row, col++);
    if (!_editMode)
        _layoutGraphs->addWidget(item.buttonDel, row, col++);
    _graphsItems.append(item);

    item.colX->setFocus();
}
