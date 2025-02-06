#include "CsvConfigDialog.h"

#include "CustomPrefs.h"
#include "core/DataReaders.h"
#include "core/DataSources.h"
#include "dialogs/OpenFileDlg.h"

#include "tools/OriSettings.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"

#include <QApplication>
#include <QCheckBox>
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
#include <QSyntaxHighlighter>
#include <QTableView>
#include <QTextStream>
#include <QSpinBox>

#define DEFAULT_COL_X 1
#define DEFAULT_COL_Y 2
#define DEFAULT_DLG_W 750
#define DEFAULT_DLG_H 550

using namespace Ori::Layouts;

//------------------------------------------------------------------------------
//                                 CsvDlgState
//------------------------------------------------------------------------------

struct CsvDlgState
{
    CsvDlgState()
    {
        root = CustomDataHelpers::loadDataSourceStates();
        file = root["file"].toObject();
        csv = root["csv"].toObject();
    }

    void applyTo(CsvConfigDialog& dlg)
    {
        dlg._valueSeparator->setText(csv["val_separators"].toString(",;"));
        dlg._valSepSpace->setChecked(csv["val_sep_space"].toBool(true));
        dlg._valSepTab->setChecked(csv["val_sep_tab"].toBool(true));
        dlg._skipFirstLines->setValue(csv["skip_first_lines"].toInt(0));
        dlg._previewLinesCount->setValue(csv["preview_lines_count"].toInt(15));
        dlg._decSepPoint->setChecked(csv["dep_sep_point"].toBool(true));
        dlg._decSepComma->setChecked(!csv["dep_sep_point"].toBool(true));

        auto jsonGraphs = csv["graphs"].toArray();
        for (auto it = jsonGraphs.begin(); it != jsonGraphs.end(); it++)
        {
            auto graphItem = (*it).toObject();
            int colX = graphItem["col_x"].toInt(0);
            int colY = graphItem["col_y"].toInt(0);
            if (colX > 0 && colY > 0)
                dlg.addGraphItem(colX, colY);
        }
    }

    void collectFrom(CsvConfigDialog& dlg)
    {
        QJsonArray jsonGraphs;
        foreach (auto item, dlg._graphsItems)
            jsonGraphs.append(QJsonObject({{"col_x", item.colX->value()},
                                           {"col_y", item.colY->value()}}));
        csv["graphs"] = jsonGraphs;
        csv["val_separators"] = dlg._valueSeparator->text().trimmed();
        csv["val_sep_space"] = dlg._valSepSpace->isChecked();
        csv["val_sep_tab"] = dlg._valSepTab->isChecked();
        csv["dep_sep_point"] = dlg._decSepPoint->isChecked();
        csv["skip_first_lines"] = dlg._skipFirstLines->value();
        csv["preview_lines_count"] = dlg._previewLinesCount->value();
    }

    void save()
    {
        root["file"] = file;
        root["csv"] = csv;
        CustomDataHelpers::saveDataSourceStates(root);
    }

    QJsonObject root, file, csv;
};

//------------------------------------------------------------------------------
//                               CsvConfigDialog
//------------------------------------------------------------------------------

CsvOpenResult CsvConfigDialog::openFile()
{
    CsvDlgState state;

    OpenFileDlg fileDlg;
    if (!fileDlg.open(&state.file)) return CsvOpenResult();

    CsvConfigDialog csvDlg;
    csvDlg._files = fileDlg.files;
    csvDlg._dataSource = fileDlg.files.size() > 1 ? QStringLiteral("{ds}")
        : QFileInfo(fileDlg.files.first()).fileName();
    state.applyTo(csvDlg);

    if (!csvDlg.exec())
        return CsvOpenResult();

    state.collectFrom(csvDlg);
    state.save();

    CsvOpenResult result;

    foreach (const QString& file, fileDlg.files)
    {
        // Some optimization - read all graphs in one pass
        // so we will return already loaded data sources
        CsvMultiReader csvReader;
        csvDlg.initReader(csvReader, file);
        QString res = csvReader.read();
        if (!res.isEmpty())
        {
            result.report << tr("Failed to read file %1: %2").arg(file, res);
            continue;
        }

        foreach (const CsvMultiReader::GraphItem& item, csvReader.graphItems)
        {
            Q_ASSERT(item.xs.size() == item.ys.size());
            if (item.xs.isEmpty()) continue;

            auto ds = new CsvFileDataSource;
            ds->_fileName = csvReader.fileName;
            ds->_params = csvReader.makeParams(item);
            ds->_data.xs = item.xs;
            ds->_data.ys = item.ys;

            result.dataSources << ds;
        }
    }

    return result;
}

CsvOpenResult CsvConfigDialog::openClipboard()
{
    CsvOpenResult result;

    QString text = qApp->clipboard()->text();
    if (text.isEmpty())
    {
        result.report << tr("Clipboard does not contain suitable data");
        return result;
    }

    CsvConfigDialog csvDlg;
    csvDlg._text = text;
    csvDlg._dlgTitle = tr("Paste as CSV");
    csvDlg._dataSource = "Clipboard";

    CsvDlgState state;
    state.applyTo(csvDlg);

    if (!csvDlg.exec())
        return result;

    state.collectFrom(csvDlg);
    state.save();

    // Some optimization - read all graphs in one pass
    // so we will return already loaded data sources
    CsvMultiReader csvReader;
    csvDlg.initReader(csvReader);
    QString res = csvReader.read();
    if (!res.isEmpty())
    {
        result.report << res;
        return result;
    }

    foreach (const CsvMultiReader::GraphItem& item, csvReader.graphItems)
    {
        Q_ASSERT(item.xs.size() == item.ys.size());
        if (item.xs.isEmpty()) continue;
        auto dataSource = new ClipboardCsvDataSource;
        dataSource->_params = csvReader.makeParams(item);
        dataSource->_data.xs = item.xs;
        dataSource->_data.ys = item.ys;
        result.dataSources << dataSource;
    }

    return result;
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

class WhitespaceHighlighter : public QSyntaxHighlighter
{
public:
    WhitespaceHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {}
protected:
    void highlightBlock(const QString& text) override {
        QTextCharFormat format;
        // TODO: adjust for dark theme
        format.setForeground(QColor(190, 190, 190));

        for (int i = 0; i < text.length(); ++i) {
            if (text[i] == ' ' || text[i] == '\t') {
                setFormat(i, 1, format);
            }
        }
    }
};

} // namespace

CsvConfigDialog::CsvConfigDialog(bool editMode) : QWidget(), _editMode(editMode)
{
    setObjectName("CsvConfigDialog");

    _fileTextPreview = new ExpandingTextEdit;
    _fileTextPreview->setReadOnly(true);
    _fileTextPreview->setWordWrapMode(QTextOption::NoWrap);
    Ori::Gui::setFontMonospace(_fileTextPreview);
    auto doc = _fileTextPreview->document();
    QTextOption option =  doc->defaultTextOption();
    option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
    doc->setDefaultTextOption(option);
    new WhitespaceHighlighter(doc);

    _dataTablePreview = new QTableView;
    _dataTablePreview->setModel(new PreviewDataModel(this));
    Ori::Gui::adjustFont(_dataTablePreview);

    _buttonShowText = new QPushButton(tr("File text"));
    _buttonShowText->setCheckable(true);
    connect(_buttonShowText, &QPushButton::clicked, this, [this](){
        _buttonShowText->setChecked(true);
        _buttonShowData->setChecked(false);
        _tabs->setCurrentIndex(0);
    });

    _buttonShowData = new QPushButton(tr("Data table"));
    _buttonShowData->setCheckable(true);
    _buttonShowData->setChecked(true);
    connect(_buttonShowData, &QPushButton::clicked, this, [this](){
        _buttonShowText->setChecked(false);
        _buttonShowData->setChecked(true);
        _tabs->setCurrentIndex(1);
    });

    _tabs = new QStackedWidget;
    _tabs->addWidget(_fileTextPreview);
    _tabs->addWidget(_dataTablePreview);
    _tabs->setCurrentIndex(1);

    _valSepSpace = new QCheckBox(tr("Space"));
    connect(_valSepSpace, &QCheckBox::clicked, this, &CsvConfigDialog::updatePreviewData);

    _valSepTab = new QCheckBox(tr("Tab"));
    connect(_valSepTab, &QCheckBox::clicked, this, &CsvConfigDialog::updatePreviewData);

    _valueSeparator = new QLineEdit;
    connect(_valueSeparator, &QLineEdit::textEdited, this, &CsvConfigDialog::updatePreviewData);

    auto layoutValueSeparators = LayoutV({ LayoutH({ _valSepSpace, _valSepTab }), _valueSeparator }).boxLayout();

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
    optionsLayout->addRow(tr("Value separators"), layoutValueSeparators);
    optionsLayout->addRow(tr("Decimal separator"), LayoutH({_decSepPoint, _decSepComma}).setMargin(0).boxLayout());
    optionsLayout->addRow(tr("Skip first lines"), _skipFirstLines);
    optionsLayout->addRow(tr("Preview lines"), _previewLinesCount);

    auto graphsGroup = new QGroupBox(tr("Graphs"));
    _layoutGraphs = new QGridLayout;
    _layoutGraphs->setContentsMargins(3, 3, 3, 3);
    _layoutGraphs->setHorizontalSpacing(6);
    _layoutGraphs->setVerticalSpacing(3);
    LayoutV({_layoutGraphs, Stretch()}).setMargin(0).useFor(graphsGroup);

    QPushButton *buttonAddGraph = nullptr;
    if (!_editMode)
    {
        buttonAddGraph = new QPushButton(tr("+ Graph"));
        connect(buttonAddGraph, &QPushButton::clicked, this, [this](){ addGraphItem(DEFAULT_COL_X, DEFAULT_COL_Y); });
    }

    LayoutV({
        LayoutH({ optionsGroup, graphsGroup })
                .setStretchFactor(optionsGroup, 1)
                .setStretchFactor(graphsGroup, 100)
                .setMargin(0),
        LayoutH({ _buttonShowText, _buttonShowData, Stretch(), buttonAddGraph}).setMargin(0),
        _tabs
    }).setMargin(0).useFor(this);

    Ori::Settings s;
    s.restoreWindowGeometry(this);
}

CsvConfigDialog::~CsvConfigDialog()
{
    Ori::Settings s;
    s.storeWindowGeometry(this);
}

bool CsvConfigDialog::exec()
{
    updateFullPreview();

    auto validator = [this](){
        if (_graphsItems.isEmpty())
            return tr("There is nothing to plot!");
        // TODO: check if some column numbers are out of bounds
        return QString();
    };

    QString title = _dlgTitle;
    if (title.isEmpty() && !_files.isEmpty())
    {
        if (_files.size() == 1)
            title = _files.first();
        else
            // TODO: add file selector for preview
            title = tr("Open %1 files").arg(_files.size());
    }

    return Ori::Dlg::Dialog(this, false)
            .withInitialSize({DEFAULT_DLG_W, DEFAULT_DLG_H})
            .withPersistenceId("csvDlg")
            .withVerification(validator)
            .withTitle(title)
            .exec();
}

void CsvConfigDialog::initReader(CsvMultiReader& reader, QString fileName)
{
    reader.text = _text;
    reader.fileName = fileName;
    reader.decimalPoint = _decSepPoint->isChecked();
    reader.skipFirstLines = _skipFirstLines->value();
    reader.valueSeparators = valueSeparators();

    foreach (const CvsGraphItemView& item, _graphsItems)
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
    updatePreviewText();
    updatePreviewData();
}

void CsvConfigDialog::updatePreviewText()
{
    _previewLines.clear();
    _fileTextPreview->clear();

    if (!_files.isEmpty())
    {
        QFile file(_files.first());
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
    LineSplitter lineSplitter(valueSeparators());
    ValueParser valueParser(_decSepPoint->isChecked());
    int maxColCount = 0;
    int lineNo = 0;
    foreach (const QString& line, _previewLines)
    {
        lineSplitter.split(line);
        QStringList rowValues;
        foreach (const auto& part, lineSplitter.parts)
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
    item.title->setText(QString("%1 [%2;%3]").arg(_dataSource).arg(colX).arg(colY));

    auto updateGraphTitle = [item](){
        QString title = item.title->text();
        static QRegularExpression reXY("^.*\\[\\s*(\\d+)\\s*[,;]\\s*(\\d+)\\s*\\].*$");
        auto m = reXY.match(title);
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
        connect(item.buttonDel, &QPushButton::clicked, this, [this, item](){
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

QString CsvConfigDialog::valueSeparators() const
{
    QString seps = _valueSeparator->text().trimmed();
    if (_valSepSpace->isChecked())
        seps += ' ';
    if (_valSepTab->isChecked())
        seps += '\t';
    return seps;
}
