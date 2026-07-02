#ifndef OPEN_FILE_DLG_H
#define OPEN_FILE_DLG_H

#include <QString>
#include <QList>

struct RecentDirState
{
    virtual QString getRecentDir() = 0;
    virtual void setRecentDir(const QString &dir) = 0;
};

struct OpenFileDlg
{
    QStringList files;

    bool open(RecentDirState* state = nullptr);
};

#endif // OPEN_FILE_DLG_H
