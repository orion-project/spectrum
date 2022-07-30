#include "OpenFileDlg.h"

#include <QApplication>
#include <QFileDialog>
#include <QJsonObject>

bool OpenFileDlg::open(QJsonObject *state)
{
    QFileDialog dlg(qApp->activeWindow());
    dlg.setFileMode(QFileDialog::ExistingFiles);

    if (items.isEmpty())
    {
        if (state)
        {
            auto dir = (*state)["dir"].toString();
            if (!dir.isEmpty())
                dlg.setDirectory(dir);
        }
    }
    else dlg.selectFile(items.first().path);

    if (dlg.exec() != QDialog::Accepted)
        return false;

    auto files = dlg.selectedFiles();
    if (files.isEmpty())
        return false;

    items.clear();
    foreach (const QString& path, files)
    {
        OpenFileItem it;
        it.path = path;
        it.name = QFileInfo(path).fileName();
        items << it;
    }

    if (state)
        (*state)["dir"] = dlg.directory().path();
    return true;
}
