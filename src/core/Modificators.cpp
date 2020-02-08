#include "Modificators.h"

#include "GraphMath.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"
#include "widgets/OriValueEdit.h"
#include "widgets/OriOptionsGroup.h"

#include <QApplication>
#include <QGroupBox>
#include <QRadioButton>

using namespace Ori::Layouts;
using namespace Ori::Gui;
using namespace GraphMath;

namespace {

Ori::Widgets::ValueEdit* makeEditor() {
    auto editor = new Ori::Widgets::ValueEdit;
    Ori::Gui::adjustFont(editor);
    return editor;
}

class RadioOptions
{
public:
    RadioOptions(QMap<int, QString> options)
    {
        auto keys = options.keys();
        for (auto key : keys)
            _buttons.insert(key, new QRadioButton(options[key]));
        _buttons[keys.first()]->setChecked(true); // TODO: restore from saved state
    }

    QRadioButton* button(int id) const { return _buttons[id]; }

    QGroupBox* makeGroup(const QString& title)
    {
        auto group = new QGroupBox(title);
        auto layout = new QVBoxLayout(group);
        for (auto key : _buttons.keys())
            layout->addWidget(_buttons[key]);
        if (_editor)
            layout->addWidget(_editor);
        return group;
    }

    template <typename TOption> TOption selection()
    {
        for (auto key : _buttons.keys())
            if (_buttons[key]->isChecked())
                return static_cast<TOption>(key);
        return static_cast<TOption>(0);
    }

    void makeEditor(int option)
    {
        _editor = ::makeEditor();
        _editor->setValue(0);
        qApp->connect(_buttons[option], &QRadioButton::clicked, [this](bool checked){
            if (checked) _editor->setFocus();
        });
        qApp->connect(_editor, &Ori::Widgets::ValueEdit::valueEdited, [this, option](){
            _buttons[option]->setChecked(true);
        });
    }

    Ori::Widgets::ValueEdit* editor() const { return _editor; }

private:
    QMap<int, QRadioButton*> _buttons;
    Ori::Widgets::ValueEdit* _editor = nullptr;
};

} // namespace

//------------------------------------------------------------------------------
//                                 Modificator
//------------------------------------------------------------------------------

Modificator::~Modificator()
{
}

//------------------------------------------------------------------------------
//                              OffsetModificator
//------------------------------------------------------------------------------

GraphResult OffsetModificator::modify(const GraphPoints &data) const
{
    return GraphResult::ok(_params.calc(data));
}

bool OffsetModificator::configure()
{
    RadioOptions direction({{ Offset::DIR_X, qApp->tr("Along X Axis") },
                            { Offset::DIR_Y, qApp->tr("Along Y Axis") }});
    RadioOptions mode({{ Offset::MODE_MAX, "X @ Y_max --> 0" },
                       { Offset::MODE_MIN, "X @ Y_min --> 0" },
                       { Offset::MODE_AVG, "X_avg --> 0" },
                       { Offset::MODE_VAL, qApp->tr("Arbitrary value") }});
    mode.makeEditor(Offset::MODE_VAL);

    qApp->connect(direction.button(Offset::DIR_X), &QRadioButton::toggled, [mode](bool alongX){
        mode.button(Offset::MODE_MAX)->setText(alongX ? "X @ Y_max --> 0" : "Y_max --> 0");
        mode.button(Offset::MODE_MIN)->setText(alongX ? "X @ Y_min --> 0" : "Y_min --> 0");
        mode.button(Offset::MODE_AVG)->setText(alongX ? "X_avg --> 0" : "Y_avg --> 0");
    });

    QWidget paramEditor;
    LayoutV({
        direction.makeGroup(qApp->tr("Direction")),
        mode.makeGroup(qApp->tr("Quantity")),
    }).setMargin(0).useFor(&paramEditor);

    if (Ori::Dlg::Dialog(&paramEditor, false)
            .withTitle(qApp->tr("Offset"))
            .withContentToButtonsSpacingFactor(2)
            .exec())
    {
        _params.dir = direction.selection<Offset::Direction>();
        _params.mode = mode.selection<Offset::Mode>();
        _params.value = mode.editor()->value();
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
//                              ScaleModificator
//------------------------------------------------------------------------------

GraphResult ScaleModificator::modify(const GraphPoints &data) const
{
    return GraphResult::ok(_params.calc(data));
}

bool ScaleModificator::configure()
{
    auto editorScaleFactor = makeEditor();
    editorScaleFactor->setValue(1);

    RadioOptions direction({{ Scale::DIR_X, qApp->tr("Along X Axis") },
                            { Scale::DIR_Y, qApp->tr("Along Y Axis") }});
    RadioOptions centerMode({{ Scale::CENTER_NON, qApp->tr("None") },
                             { Scale::CENTER_MAX, qApp->tr("Maximum") },
                             { Scale::CENTER_MIN, qApp->tr("Minimum") },
                             { Scale::CENTER_AVG, qApp->tr("Average") },
                             { Scale::CENTER_VAL, qApp->tr("Arbitrary value") }});
    centerMode.makeEditor(Scale::CENTER_VAL);

    QWidget paramEditor;
    LayoutV({
        direction.makeGroup(qApp->tr("Direction")),
        centerMode.makeGroup(qApp->tr("Scale Center")),
        groupV(qApp->tr("Factor"), { editorScaleFactor })
    }).setMargin(0).useFor(&paramEditor);

    if (Ori::Dlg::Dialog(&paramEditor, false)
            .withTitle(qApp->tr("Scale"))
            .withContentToButtonsSpacingFactor(2)
            .exec())
    {
        _params.dir = direction.selection<Scale::Direction>();
        _params.centerMode = centerMode.selection<Scale::CenterMode>();
        _params.centerValue = centerMode.editor()->value();
        _params.scaleFactor = editorScaleFactor->value();
        return true;
    }
    return false;
}
