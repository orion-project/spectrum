#include "OpenFileDlg.h"

#include <QApplication>
#include <QFileDialog>

bool OpenFileDlg::open(RecentDirState *state)
{
    QFileDialog dlg(qApp->activeWindow());
    dlg.setFileMode(QFileDialog::ExistingFiles);

    if (files.isEmpty())
    {
        if (state)
        {
            auto dir = state->getRecentDir();
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
        state->setRecentDir(dlg.directory().path());

    return true;
}
