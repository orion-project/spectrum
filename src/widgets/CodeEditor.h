#ifndef CODE_EDITOR_H
#define CODE_EDITOR_H

#include <QWidget>

#include "core/BaseTypes.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
QT_END_NAMESPACE

namespace Ori::Widgets
{
class CodeEditor;
}

class CodeEditor : public QWidget
{
    Q_OBJECT

public:
    CodeEditor();
    
    QString code() const;
    void setCode(const QString &code);
    
    GraphResult verify();
    
private:
    Ori::Widgets::CodeEditor *_editor;
    QPlainTextEdit* _log;
    QStringList _presets;
    QMenu *_menuPresets;
    QAction *_actnSavePreset;
    
    enum LogLevel { INFO, ERROR };
    void logLine(const QString &msg, LogLevel level = INFO);
    void clearLog();
    void savePreset();
    void updatePesetsMenu();
    void resetPresets();
};

#endif // CODE_EDITOR_H