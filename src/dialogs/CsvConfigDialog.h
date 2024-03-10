#ifndef CSV_CONFIG_DIALOG_H
#define CSV_CONFIG_DIALOG_H

#include "core/OriResult.h"

#include <QWidget>

class DataSource;
class CsvFileDataSource;
struct CsvDlgState;
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

struct CsvOpenResult
{
    QStringList report;
    QVector<DataSource*> dataSources;
};

class CsvConfigDialog : public QWidget
{
    Q_OBJECT

public:
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

    QString _dlgTitle, _text, _dataSource;
    QStringList _files, _previewLines;
    QStackedWidget *_tabs;
    QPlainTextEdit *_fileTextPreview;
    QTableView *_dataTablePreview;
    QPushButton *_buttonShowText, *_buttonShowData;
    QSpinBox *_skipFirstLines, *_previewLinesCount;
    QLineEdit *_valueSeparator;
    QGridLayout *_layoutGraphs;
    QList<CvsGraphItemView> _graphsItems;
    QRadioButton *_decSepPoint, *_decSepComma;
    bool _editMode = false;

    explicit CsvConfigDialog(bool editMode = false);

    bool exec();

    void updateFullPreview();
    void updatePreviewText();
    void updatePreviewText(QTextStream& stream);
    void updatePreviewData();
    void addNewGraph();
    void addGraphItem(int colX, int colY);
    void initReader(CsvMultiReader& reader, QString fileName = QString());

    friend struct CsvDlgState;
};

#endif // CSV_CONFIG_DIALOG_H
