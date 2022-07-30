#ifndef OPEN_FILE_DLG_H
#define OPEN_FILE_DLG_H

#include <QString>
#include <QList>

QT_BEGIN_NAMESPACE
class QJsonObject;
QT_END_NAMESPACE

struct OpenFileItem
{
    QString path;
    QString name;
};

struct OpenFileDlg
{
    QList<OpenFileItem> items;

    bool open(QJsonObject* state = nullptr);
};

#endif // OPEN_FILE_DLG_H
