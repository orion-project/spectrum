#include "OpenFileDlg.h"

#include <QApplication>
#include <QFileDialog>
#include <QJsonObject>

bool OpenFileDlg::open(QJsonObject *state)
{
    QFileDialog dlg(qApp->activeWindow());
    dlg.setFileMode(QFileDialog::ExistingFiles);

    if (files.isEmpty())
    {
        if (state)
        {
            auto dir = (*state)["dir"].toString();
            if (!dir.isEmpty())
                dlg.setDirectory(dir);
        }
    }
    else dlg.selectFile(files.first());

    if (dlg.exec() != QDialog::Accepted)
        return false;

    files = dlg.selectedFiles();
    if (files.isEmpty())
        return false;

    if (state)
        (*state)["dir"] = dlg.directory().path();

    return true;
}
