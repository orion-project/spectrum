#include "CsvConfigDialog.h"

#include "CustomPrefs.h"
#include "core/DataReaders.h"
#include "core/DataSources.h"

#include "helpers/OriDialogs.h"
#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"

#include <QApplication>
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

using namespace Ori::Layouts;

CsvOpenResult CsvConfigDialog::openFile()
{
    auto root = CustomDataHelpers::loadCustomData("datasources");
    auto stateFile = root["file"].toObject();

    QFileDialog fileDlg(qApp->activeWindow());
    auto dir = stateFile["dir"].toString();
    if (!dir.isEmpty())
        fileDlg.setDirectory(dir);

    QVector<DataSource*> dataSources;
    if (fileDlg.exec() != QDialog::Accepted)
        return CsvOpenResult::ok(dataSources);
    auto files = fileDlg.selectedFiles();
    if (files.isEmpty())
        return CsvOpenResult::ok(dataSources);
    auto fileName = files.first();
    if (fileName.isEmpty())
        return CsvOpenResult::ok(dataSources);

    CsvConfigDialog paramsEditor;
    auto stateCsv = root["csv_file"].toObject();
    paramsEditor._valueSeparator->setText(stateCsv["value_separator"].toString(",;"));
    paramsEditor._skipFirstLines->setValue(stateCsv["skip_first_lines"].toInt(0));
    paramsEditor._previewLinesCount->setValue(stateCsv["preview_lines_count"].toInt(15));
    paramsEditor._decSepPoint->setChecked(stateCsv["dep_sep_point"].toBool(true));
    paramsEditor._decSepComma->setChecked(!stateCsv["dep_sep_point"].toBool(true));
    paramsEditor._fileName = fileName;
    paramsEditor._graphBaseName = QFileInfo(fileName).fileName();
    paramsEditor.updateFullPreview();

    auto jsonGraphs = stateCsv["graphs"].toArray();
    for (auto it = jsonGraphs.begin(); it != jsonGraphs.end(); it++)
    {
        auto graphItem = (*it).toObject();
        int colX = graphItem["col_x"].toInt(0);
        int colY = graphItem["col_y"].toInt(0);
        if (colX > 0 && colY > 0)
            paramsEditor.addGraphItem(colX, colY);
    }

    auto validator = [&paramsEditor](){
        if (paramsEditor._graphsItems.isEmpty())
            return tr("There is nothing to plot!");
        return QString();
    };

    auto dlg = Ori::Dlg::Dialog(&paramsEditor, false)
            .withInitialSize({stateCsv["window_width"].toInt(750),
                              stateCsv["window_height"].toInt(550)})
            .withVerification(validator)
            .withTitle(fileName);
    if (!dlg.exec())
        return CsvOpenResult::ok(dataSources);

    jsonGraphs = QJsonArray();
    CsvMultiReader csvReader;
    for (const CvsGraphItemView& item : paramsEditor._graphsItems)
    {
        CsvMultiReader::GraphItem it;
        it.columnX = item.colX->value();
        it.columnY = item.colY->value();
        it.title = item.title->text().trimmed();
        jsonGraphs.append(QJsonObject({{"col_x", it.columnX}, {"col_y", it.columnY}}));
        csvReader.graphItems.append(it);
    }
    csvReader.fileName = fileName;
    csvReader.valueSeparators = paramsEditor._valueSeparator->text().trimmed();
    csvReader.decimalPoint = paramsEditor._decSepPoint->isChecked();
    csvReader.skipFirstLines = paramsEditor._skipFirstLines->value();

    stateFile["dir"] = fileDlg.directory().path();
    stateCsv["window_width"] = dlg.size().width();
    stateCsv["window_height"] = dlg.size().height();
    stateCsv["value_separator"] = csvReader.valueSeparators;
    stateCsv["dep_sep_point"] = csvReader.decimalPoint;
    stateCsv["skip_first_lines"] = csvReader.skipFirstLines;
    stateCsv["preview_lines_count"] = paramsEditor._previewLinesCount->value();
    stateCsv["graphs"] = jsonGraphs;
    root["file"] = stateFile;
    root["csv_file"] = stateCsv;
    CustomDataHelpers::saveCustomData(root, "datasources");

    QString res = csvReader.read();
    if (!res.isEmpty())
        return CsvOpenResult::fail(res);

    for (const CsvMultiReader::GraphItem& item : csvReader.graphItems)
    {
        if (item.xs.isEmpty()) continue;
        if (item.xs.size() != item.ys.size()) continue;
        auto dataSource = new CsvFileDataSource;
        dataSource->_fileName = fileName;
        dataSource->_title = item.title;
        dataSource->_columnX = item.columnX;
        dataSource->_columnY = item.columnY;
        dataSource->_valueSeparators = csvReader.valueSeparators;
        dataSource->_decimalPoint = csvReader.decimalPoint;
        dataSource->_skipFirstLines = csvReader.skipFirstLines;
        dataSource->_initialData.xs = item.xs;
        dataSource->_initialData.ys = item.ys;
        dataSources << dataSource;
    }
    return CsvOpenResult::ok(dataSources);
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

CsvConfigDialog::CsvConfigDialog(QWidget *parent) : QWidget(parent)
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

    auto buttonAddGraph = new QPushButton(tr("+ Graph"));
    connect(buttonAddGraph, &QPushButton::clicked, [this](){ addGraphItem(DEFAULT_COL_X, DEFAULT_COL_Y); });

    LayoutV({
        LayoutH({ optionsGroup, graphsGroup })
                .setStretchFactor(optionsGroup, 1)
                .setStretchFactor(graphsGroup, 100)
                .setMargin(0),
        LayoutH({ _buttonShowText, _buttonShowData, Stretch(), buttonAddGraph}).setMargin(0),
        _tabs
    }).setMargin(0).useFor(this);
}

void CsvConfigDialog::updateFullPreview()
{
    if (_fileName.isEmpty()) return;

    updatePreviewText();
    updatePreviewData();
}

void CsvConfigDialog::updatePreviewText()
{
    _previewLines.clear();
    _fileTextPreview->clear();

    QFile file(_fileName);
    bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!ok)
    {
        _fileTextPreview->setPlainText(qApp->tr("Failed to read the file: %2").arg(file.errorString()));
        return;
    }

    QTextStream stream(&file);
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

void CsvConfigDialog::addGraphItem(int colX, int colY)
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
    item.title->setText(QString("%1 [%2;%3]").arg(_graphBaseName).arg(colX).arg(colY));

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

    int col = 0;
    _layoutGraphs->addWidget(item.labX, row, col++);
    _layoutGraphs->addWidget(item.colX, row, col++);
    _layoutGraphs->addWidget(item.labY, row, col++);
    _layoutGraphs->addWidget(item.colY, row, col++);
    _layoutGraphs->addWidget(item.labTitle, row, col++);
    _layoutGraphs->addWidget(item.title, row, col++);
    _layoutGraphs->addWidget(item.buttonDel, row, col++);
    _graphsItems.append(item);

    item.colX->setFocus();
}
