#ifndef HELP_WINDOW_H
#define HELP_WINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QTextBrowser;
QT_END_NAMESPACE

class HelpWindow : public QWidget
{
    Q_OBJECT

public:
    static void showContent();
    static void showTopic(const QString& topic);

private:
    static void openWindow();

    explicit HelpWindow();
    ~HelpWindow();

    QTextBrowser *_browser;

    void setSource(const QString& name);
    void editStylesheet();
};

#endif // HELP_WINDOW_H
