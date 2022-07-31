#ifndef OPEN_FILE_DLG_H
#define OPEN_FILE_DLG_H

#include <QString>
#include <QList>

QT_BEGIN_NAMESPACE
class QJsonObject;
QT_END_NAMESPACE

struct OpenFileDlg
{
    QStringList files;

    bool open(QJsonObject* state = nullptr);
};

#endif // OPEN_FILE_DLG_H
