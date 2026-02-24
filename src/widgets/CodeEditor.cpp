#include "CodeEditor.h"

#include "app/HelpSystem.h"
#include "app/PersistentState.h"
#include "core/DataSources.h"

#include "helpers/OriDialogs.h"
#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"
#include "tools/OriHighlighter.h"
#include "widgets/OriCodeEditor.h"
#include "widgets/OriFlatToolBar.h"

#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QWidgetAction>

using namespace Ori::Gui;
using namespace Ori::Layouts;

CodeEditor::CodeEditor() : QWidget()
{
    _editor = new Ori::Widgets::CodeEditor;
    setFontMonospace(_editor);
    _editor->setShowWhitespaces(true);
    _editor->setTabWidth(2);
    Ori::Highlighter::setHighlighter(_editor, ":/syntax/lua");

    _actnSavePreset = action(tr("Save Current Code as Preset"), this, &CodeEditor::savePreset, ":/toolbar/star");
    _actnSavePreset->setIconText(tr("Save Preset"));

    _menuPresets = new QMenu(this);
    _menuPresets->addAction(_actnSavePreset);
    connect(_menuPresets, &QMenu::aboutToShow, this, &CodeEditor::updatePesetsMenu);

    auto butSavePreset = new QToolButton;
    butSavePreset->setDefaultAction(_actnSavePreset);
    butSavePreset->setPopupMode(QToolButton::MenuButtonPopup);
    butSavePreset->setMenu(_menuPresets);

    auto toolbar = new Ori::Widgets::FlatToolBar;
    populate(toolbar, {
        textToolButton(action(tr("Check Formula"), this, &CodeEditor::verify, ":/toolbar/check", "Ctrl+Return")),
        action(tr("Clear Log"), this, &CodeEditor::clearLog, ":/toolbar/clear_log"),
        0,
        butSavePreset,
        0,
        action(tr("Help"), this, []{ Z::HelpSystem::topic("add_formula"); }, ":/toolbar/help", "F1"),
    });
    
    _log = new QPlainTextEdit;
    setFontMonospace(_log);
    _log->setReadOnly(true);
    _log->setUndoRedoEnabled(false);
    _log->setTabStopDistance(24);
    
    auto splitter = splitterV(_editor, 3, _log, 1);
    
    LayoutV({ toolbar, LayoutH({ SpaceH(), splitter, SpaceH() }) }).setSpacing(0).setMargin(0).useFor(this);
}
  
QString CodeEditor::code() const
{
    return _editor->code();
}

void CodeEditor::setCode(const QString &code)
{
    _editor->setCode(code);
}
 
GraphResult CodeEditor::verify()
{
    auto res = FormulaDataSource::exec(_editor->code());
    if (res.ok())
    {
        auto [xs, ys] = res.result();
        logLine(QStringLiteral("Executed successfully. Points: %1").arg(xs.size()));
        QStringList strX, strY;
        if (xs.size() < 10)
        {
            for (int i = 0; i < xs.size(); i++)
            {
                strX << QString::number(xs.at(i));
                strY << QString::number(ys.at(i));
            }
        }
        else
        {
            for (int i = 0; i < 3; i++)
            {
                strX << QString::number(xs.at(i));
                strY << QString::number(ys.at(i));
            }
            strX << QStringLiteral("...");
            strY << QStringLiteral("...");
            for (int i = xs.size()-3; i < xs.size(); i++)
            {
                strX << QString::number(xs.at(i));
                strY << QString::number(ys.at(i));
            }
        }
        logLine(QStringLiteral("X: [%1]").arg(strX.join(", ")));
        logLine(QStringLiteral("Y: [%1]").arg(strY.join(", ")));
    }
    else
        logLine(res.error(), ERROR);
    return res;
}

void CodeEditor::logLine(const QString &msg, LogLevel level)
{
    qDebug() << msg;
    
    _log->appendPlainText(msg);

    // Format the last paragraph
    QTextCursor cursor = _log->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    QTextCharFormat format;
    format.setForeground(level == ERROR ? Qt::red : QColor(0xff222244));
    cursor.mergeCharFormat(format);
    
    // scroll to end
    cursor.movePosition(QTextCursor::End);
    _log->setTextCursor(cursor);
    _log->ensureCursorVisible();
}

void CodeEditor::clearLog()
{
    _log->clear();
}

struct FormulaPresets
{
    FormulaPresets()
    {
        _root = PersistentState::load("formula_presets");
    }
    
    ~FormulaPresets()
    {
        if (_changed)
            PersistentState::save("formula_presets", _root);
    }
    
    QStringList getAll() const
    {
        QStringList names;
        for (auto it = _root.constBegin(); it != _root.constEnd(); it++)
            names << it.key();
        names.sort();
        return names;
    }
    
    QString get(const QString &name)
    {
        return _root[name].toString();
    }
    
    bool add(std::function<QString()> getCode)
    {
        QString name = Ori::Dlg::inputText(qApp->translate("Formula", "Preset name"), {}).trimmed();
        if (name.isEmpty())
           return false;
        if (_root.contains(name))
            if (!Ori::Dlg::yes(qApp->translate("Formula", "A preset with the same name already exists, overwrite?")))
                return false;
        _root[name] = getCode();
        _changed = true;
        return true;
    }
    
    bool remove(const QString &name)
    {
        if (!Ori::Dlg::yes(qApp->translate("Formula", "Remove preset <b>%1</b> ?").arg(name)))
            return false;
        _root.remove(name);
        _changed = true;
        return true;
    }
    
private:
    QJsonObject _root;
    bool _changed = false;
};

void CodeEditor::savePreset()
{
    if (FormulaPresets().add([this]{ return _editor->code(); }))
        resetPresets();
}

void CodeEditor::updatePesetsMenu()
{
    if (!_presets.isEmpty())
        return;
        
    _presets = FormulaPresets().getAll();
    
    for (const auto &preset : std::as_const(_presets))
    {
        auto action = new QWidgetAction(_menuPresets);
        auto button = new QToolButton;
        auto menu = new QMenu(button);
        
        auto actnReplace = menu->addAction(QIcon(":/toolbar/page"), tr("Replace Current Code"), this, [this, preset]{
            _menuPresets->close();
            _editor->setCode(FormulaPresets().get(preset));
            QTimer::singleShot(0, this, [this]{ _editor->setFocus(); });
        });
        
        menu->addAction(QIcon(":/toolbar/page_insert"), tr("Insert Into Code"), this, [this, preset]{
            _menuPresets->close();
            _editor->textCursor().insertText(FormulaPresets().get(preset));
            QTimer::singleShot(0, this, [this]{ _editor->setFocus(); });
        });
        
        menu->addAction(QIcon(":/toolbar/delete"), tr("Remove Preset..."), this, [this, preset]{
            if (FormulaPresets().remove(preset))
                resetPresets();
        });
        
        auto actnApply = new QAction(preset, button);
        connect(actnApply, &QAction::triggered, actnReplace, &QAction::trigger);
        
        button->setDefaultAction(actnApply);
        button->setMenu(menu);
        button->setPopupMode(QToolButton::MenuButtonPopup);
        button->setProperty("role", "preset-menu-item");

        action->setDefaultWidget(button);
        _menuPresets->addAction(action);
    }
}

void CodeEditor::resetPresets()
{
    _presets.clear();
    _menuPresets->clear();
    // There must be something in the menu to make it "popupable"
    _menuPresets->addAction(_actnSavePreset);
    _menuPresets->addSeparator();
}
