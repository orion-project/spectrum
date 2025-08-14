#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <QString>

class QObject;

namespace FileUtils
{

QString filtersForOpen();
QString filtersForSave();
inline QString suffix() { return QStringLiteral("spectr"); }
QString refineFileName(const QString& fileName, const QString &selectedFilter);
QString appendSuffix(const QString& fileName, const QString &selectedSuffix);
QString extractSuffix(const QString& filter);
QString getProjectOpenFileName(QObject *parent);
QString getProjectSaveFileName(QObject *parent);

} // namespace FileUtils

#endif // FILE_UTILS_H
