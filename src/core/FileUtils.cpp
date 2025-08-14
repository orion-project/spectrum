#include "FileUtils.h"

#include "tools/OriSettings.h"

#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>

namespace FileUtils
{

QString filtersForProject()
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

QString getProjectOpenFileName(QObject *parent)
{
    Ori::Settings s;
    s.beginGroup("Recent");

    QString recentPath = s.strValue("prjSavePath");
    QString recentFilter = s.strValue("prjSaveFilter");

    auto fileName = QFileDialog::getOpenFileName(qobject_cast<QWidget*>(parent),
                                                 qApp->tr("Open Project", "Dialog title"),
                                                 recentPath,
                                                 filtersForProject(),
                                                 &recentFilter);
    if (fileName.isEmpty()) return QString();

    s.setValue("prjSavePath", fileName);
    s.setValue("prjSaveFilter", recentFilter);

    return refineFileName(fileName, recentFilter);
}

QString getProjectSaveFileName(QObject *parent)
{
    Ori::Settings s;
    s.beginGroup("Recent");

    QString recentPath = s.strValue("prjSavePath");
    QString recentFilter = s.strValue("prjSaveFilter");

    auto fileName = QFileDialog::getSaveFileName(qobject_cast<QWidget*>(parent),
                                                 qApp->tr("Save Project", "Dialog title"),
                                                 recentPath,
                                                 filtersForProject(),
                                                 &recentFilter);
    if (fileName.isEmpty()) return QString();

    s.setValue("prjSavePath", fileName);
    s.setValue("prjSaveFilter", recentFilter);

    return refineFileName(fileName, recentFilter);
}


} // namespace FileUtils
