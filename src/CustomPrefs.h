#ifndef CUSTOM_PREFS_H
#define CUSTOM_PREFS_H

#include <QJsonObject>
#include <QSize>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace CustomDataHelpers {

QJsonObject loadCustomData(const QString& spec);
void saveCustomData(const QJsonObject& root, const QString& spec);

inline QJsonObject loadDataSourceStates() { return loadCustomData("datasources"); }
inline void saveDataSourceStates(const QJsonObject& states) { saveCustomData(states, "datasources"); }

void storeWindowSize(QJsonObject& root, QWidget* wnd);
void restoreWindowSize(const QJsonObject& root, QWidget* wnd);
void restoreWindowSize(const QJsonObject& root, QWidget* wnd, int defaultW, int defaultH);

} // namespace CustomData

class CustomPrefs
{
public:
    CustomPrefs() = delete;

    static void load(const QString& appConfigFile);

    static void setRecentDir(const QString& key, const QString& dirOrFile);
    static QString recentDir(const QString& key, const QString& defaultDir = QString());
    static void setRecentStr(const QString& key, const QString& value);
    static QString recentStr(const QString& key, const QString& defaultStr = QString());
    static void setRecentObj(const QString& key, const QJsonObject& obj);
    static QJsonObject recentObj(const QString& key);
    static QSize recentSize(const QString& key, const QSize &defaultSize = QSize());
    static void setRecentSize(const QString& key, const QSize& size);
};

#endif // CUSTOM_PREFS_H
