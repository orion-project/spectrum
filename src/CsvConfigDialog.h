#ifndef CSV_CONFIG_DIALOG_H
#define CSV_CONFIG_DIALOG_H

#include "core/OriResult.h"

#include <QWidget>

class DataSource;

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
QT_END_NAMESPACE

using CsvOpenResult = Ori::Result<QVector<DataSource*>>;

class CsvConfigDialog : public QWidget
{
    Q_OBJECT

public:
    explicit CsvConfigDialog(QWidget *parent = nullptr);

    static CsvOpenResult openFile();

private:
    struct CvsGraphItemView
    {
        QLabel *labX, *labY, *labTitle;
        QSpinBox *colX, *colY;
        QLineEdit *title;
        QPushButton *buttonDel;
    };

    QString _fileName, _graphBaseName;
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
    void updatePreviewData();
    void addNewGraph();
    void addGraphItem(int colX, int colY);
};

#endif // CSV_CONFIG_DIALOG_H
