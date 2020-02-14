#ifndef CSV_CONFIG_DIALOG_H
#define CSV_CONFIG_DIALOG_H

#include "core/OriResult.h"

#include <QWidget>

class DataSource;
struct CsvOpenerState;
struct CsvMultiReader;

QT_BEGIN_NAMESPACE
class QGridLayout;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QStackedWidget;
class QTableView;
class QTextStream;
QT_END_NAMESPACE

using CsvOpenResult = Ori::Result<QVector<DataSource*>>;

class CsvConfigDialog : public QWidget
{
    Q_OBJECT

public:
    explicit CsvConfigDialog(QWidget *parent = nullptr);

    static CsvOpenResult openFile();
    static CsvOpenResult openClipboard();

private:
    struct CvsGraphItemView
    {
        QLabel *labX, *labY, *labTitle;
        QSpinBox *colX, *colY;
        QLineEdit *title;
        QPushButton *buttonDel;
    };

    QString _fileName, _text, _graphBaseName;
    QStringList _previewLines;
    QStackedWidget *_tabs;
    QPlainTextEdit *_fileTextPreview;
    QTableView *_dataTablePreview;
    QPushButton *_buttonShowText, *_buttonShowData;
    QSpinBox *_skipFirstLines, *_previewLinesCount;
    QLineEdit *_valueSeparator;
    QGridLayout *_layoutGraphs;
    QList<CvsGraphItemView> _graphsItems;
    QRadioButton *_decSepPoint, *_decSepComma;

    void updateFullPreview();
    void updatePreviewText();
    void updatePreviewText(QTextStream& stream);
    void updatePreviewData();
    void addNewGraph();
    void addGraphItem(int colX, int colY);
    bool showDialog(const QString& title, CsvOpenerState& state);
    void initReader(CsvMultiReader& reader);

    friend struct CsvOpenerState;
};

#endif // CSV_CONFIG_DIALOG_H
