#ifndef Z_HELP_SYSTEM_H
#define Z_HELP_SYSTEM_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
QT_END_NAMESPACE

namespace Z {

class HelpSystem : public QObject
{
    Q_OBJECT

public:
    static HelpSystem* instance();

    static QString appVersion();

    static void topic(const QString& topic) { instance()->showTopic(topic); }

public slots:
    void showContent();
    void showTopic(const QString& topic);
    void visitHomePage();
    void sendBugReport();
    void checkUpdates();
    void showAbout();

private:
    HelpSystem();

    QNetworkAccessManager* _updateChecker = nullptr;
    QNetworkReply* _updateReply = nullptr;

    void versionReceived(QByteArray versionData) const;
};

} // namespace Z

#endif // Z_HELP_SYSTEM_H
