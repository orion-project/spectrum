#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <QString>

namespace FileUtils
{

QString filtersForOpen();
QString filtersForSave();
inline QString suffix() { return QStringLiteral("spectr"); }
QString refineFileName(const QString& fileName, const QString &selectedFilter);
QString appendSuffix(const QString& fileName, const QString &selectedSuffix);
QString extractSuffix(const QString& filter);

} // namespace FileUtils

#endif // FILE_UTILS_H
