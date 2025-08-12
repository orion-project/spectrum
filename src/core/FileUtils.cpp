#include "FileUtils.h"

#include <QApplication>
#include <QFileInfo>

namespace FileUtils
{

QString filtersForOpen()
{
    return qApp->translate("IO",
                           "Spectrum project files (*.%1)\n"
                           "All files (*.*)")
            .arg(suffix());
}

QString filtersForSave()
{
    return qApp->translate("IO",
                           "Spectrum project files (*.%1)\n"
                           "All files (*.*)")
            .arg(suffix());
}

QString refineFileName(const QString& fileName, const QString &selectedFilter)
{
    return appendSuffix(fileName, extractSuffix(selectedFilter));
}

QString appendSuffix(const QString& fileName, const QString& selectedSuffix)
{
    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix().isEmpty())
    {
        auto ext = selectedSuffix.isEmpty()? suffix(): selectedSuffix;

        if (fileName.endsWith('.'))
            return fileName % ext;

        return fileName % '.' % ext;
    }
    return fileName;
}

QString extractSuffix(const QString& filter)
{
    if (filter.isEmpty()) return suffix();

    int start = filter.indexOf('.');
    if (start < 0) return suffix();

    int stop = filter.indexOf(')');
    if (stop < 0) return suffix();

    auto ext = filter.mid(start+1, stop-start-1);
    if (ext == QStringLiteral("*")) return suffix();

    return ext;
}

} // namespace FileUtils
