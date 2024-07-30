#include "Modifiers.h"

#include "CustomPrefs.h"
#include "app/HelpSystem.h"
#include "core/GraphMath.h"

#include "helpers/OriDialogs.h"
#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"
#include "widgets/OriValueEdit.h"

#include <QApplication>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>

using namespace Ori::Layouts;
using namespace Ori::Gui;
using namespace GraphMath;
using Ori::Widgets::ValueEdit;

namespace {

ValueEdit* makeEditor() {
    auto editor = new ValueEdit;
    Ori::Gui::adjustFont(editor);
    return editor;
}

class ValueOption : public QGroupBox
{
public:
    ValueOption(const QString& title) : QGroupBox(title)
    {
        setLayout(new QVBoxLayout);
        layout()->addWidget(_editor = makeEditor());
    }

    double value() const { return _editor->value(); }

    void setValue(const QJsonValue &val, double def = 1)
    {
        _editor->setValue(val.toDouble(def));
    }

private:
    ValueEdit* _editor;
};

template <typename TOption> class RadioOptions : public QGroupBox
{
public:
    RadioOptions(const QString &title, std::initializer_list<QPair<TOption, QString>> opts) : QGroupBox(title)
    {
        setLayout(new QVBoxLayout);
        for (auto opt = opts.begin(); opt != opts.end(); opt++) {
            auto but = new QRadioButton(opt->second);
            _buttons.insert(opt->first, but);
            layout()->addWidget(but);
        }
        _buttons[opts.begin()->first]->setChecked(true);
    }

    QRadioButton* button(int id) const { return _buttons[id]; }

    TOption selection()
    {
        for (auto key = _buttons.keyBegin(); key != _buttons.keyEnd(); key++)
            if (_buttons[*key]->isChecked())
                return static_cast<TOption>(*key);
        return static_cast<TOption>(0);
    }

    void setSelection(const QJsonValue &id, int def = 0)
    {
        _buttons[id.toInt(def)]->setChecked(true);
    }

    void setEditorValue(const QJsonValue &val, double def = 1)
    {
        if (_editor)
            _editor->setValue(val.toDouble(def));
    }

    void makeEditor(int option)
    {
        _editor = ::makeEditor();
        _editor->setValue(0);
        connect(_buttons[option], &QRadioButton::clicked, this, [this](bool checked){
            if (checked) _editor->setFocus();
        });
        connect(_editor, &ValueEdit::valueEdited, this, [this, option](){
            _buttons[option]->setChecked(true);
        });
        layout()->addWidget(_editor);
    }

    Ori::Widgets::ValueEdit* editor() const { return _editor; }

private:
    QMap<int, QRadioButton*> _buttons;
    Ori::Widgets::ValueEdit* _editor = nullptr;
};

class AxisOption : public RadioOptions<Direction>
{
public:
    AxisOption() : RadioOptions<Direction>(qApp->tr("Direction"), {
        { DIR_X, qApp->tr("Along X Axis" )},
        { DIR_Y, qApp->tr("Along Y Axis" )},
    }) {}
};

class IntervalOption : public QGroupBox
{
public:
    IntervalOption(const QString &title) : QGroupBox(title)
    {
        auto layout = new QGridLayout;
        setLayout(layout);
        _usePoints = new QRadioButton(tr("Points"));
        _useStep = new QRadioButton(tr("Value"));
        _points = new QSpinBox;
        _points->setRange(1, std::numeric_limits<int>::max());
        _step = makeEditor();
        layout->addWidget(_usePoints, 0, 0);
        layout->addWidget(_points, 0, 1);
        layout->addWidget(_useStep, 1, 0);
        layout->addWidget(_step, 1, 1);
        connect(_usePoints, &QRadioButton::clicked, this, [this](bool checked){ if (checked) _points->setFocus(); });
        connect(_useStep, &QRadioButton::clicked, this, [this](bool checked){ if (checked) _step->setFocus(); });
        connect(_points, qOverload<int>(&QSpinBox::valueChanged), this, [this](){ _usePoints->setChecked(true); });
        connect(_step, &ValueEdit::valueEdited, this, [this](){ _useStep->setChecked(true); });
    }

    int points() const { return _points->value(); }
    void setPoints(const QJsonValue& val, int def) { _points->setValue(val.toInt(def)); }

    double step() const { return _step->value(); }
    void setStep(const QJsonValue& val, double def) { _step->setValue(val.toDouble(def)); }

    bool useStep() const { return _useStep->isChecked(); }
    void setUseStep(const QJsonValue& val) { (val.toBool() ? _useStep : _usePoints)->setChecked(true); }

private:
    QRadioButton *_usePoints, *_useStep;
    QSpinBox *_points;
    ValueEdit *_step;
};

struct State
{
    State(const char *name) : name(name) {
        root = CustomDataHelpers::loadCustomData("modificators");
        state = root[name].toObject();
    }

    ~State() {
        if (modified) {
            root[name] = state;
            CustomDataHelpers::saveCustomData(root, "modificators");
        }
    }

    QJsonValue operator[](const char *key) const { return state[key]; }
    QJsonValueRef operator[](const char *key) { modified = true; return state[key]; }

    const char *name;
    QJsonObject root, state;
    bool modified = false;
};

bool dlg(const QString &title, std::initializer_list<LayoutItem> items, const QString &helpTopic, std::function<void()> apply)
{
    Q_UNUSED(helpTopic)
    auto editor = LayoutV(items).setMargin(0).makeWidgetAuto();
    auto ok = Ori::Dlg::Dialog(editor)
        .windowModal()
        .withTitle(title)
        // .withOnHelp([helpTopic]{ Z::HelpSystem::topic(helpTopic); })
        // .withHelpIcon(":/toolbar/help")
        .withContentToButtonsSpacingFactor(2)
        .exec();
    if (ok)
        apply();
    return ok;
}

} // namespace

//------------------------------------------------------------------------------
//                              OffsetModifier
//------------------------------------------------------------------------------

bool OffsetModifier::configure()
{
    auto dir = new AxisOption;
    auto mode = new RadioOptions<Offset::Mode>(qApp->tr("Quantity"),
        {{ Offset::MODE_MAX, "X_max --> 0" },
         { Offset::MODE_MIN, "X_min --> 0" },
         { Offset::MODE_AVG, "X_avg --> 0" },
         { Offset::MODE_MID, "X_med --> 0" },
         { Offset::MODE_VAL, qApp->tr("Arbitrary value") }});
    mode->makeEditor(Offset::MODE_VAL);

    dir->connect(dir->button(DIR_X), &QRadioButton::toggled, dir, [mode](bool alongX){
        mode->button(Offset::MODE_MAX)->setText(alongX ? "X_max --> 0" : "Y_max --> 0");
        mode->button(Offset::MODE_MIN)->setText(alongX ? "X_min --> 0" : "Y_min --> 0");
        mode->button(Offset::MODE_AVG)->setText(alongX ? "X_avg --> 0" : "Y_avg --> 0");
        mode->button(Offset::MODE_MID)->setText(alongX ? "X_med --> 0" : "Y_med --> 0");
    });

    State state("offset");
    dir->setSelection(state["dir"]);
    mode->setSelection(state["mode"]);
    mode->setEditorValue(state["value"], 0);

    return dlg(qApp->tr("Offset"), {dir, mode}, "offset", [&]{
        state["dir"] = _params.dir = dir->selection();
        state["mode"] = _params.mode = mode->selection();
        state["value"] = _params.value = mode->editor()->value();
    });
}

//------------------------------------------------------------------------------
//                              FlipModifier
//------------------------------------------------------------------------------

bool FlipModifier::configure()
{
    auto dir = new AxisOption;
    auto mode = new RadioOptions<Flip::Mode>(qApp->tr("Flip Center"),
        {{ Flip::CENTER_ZERO, qApp->tr("Zero") },
         { Flip::CENTER_MAX, qApp->tr("Maximum") },
         { Flip::CENTER_MIN, qApp->tr("Minimum") },
         { Flip::CENTER_AVG, qApp->tr("Average") },
         { Flip::CENTER_MID, qApp->tr("Median") },
         { Flip::CENTER_VAL, qApp->tr("Arbitrary value") }});
    mode->makeEditor(Flip::CENTER_VAL);

    State state("flip");
    dir->setSelection(state["dir"]);
    mode->setSelection(state["centerMode"]);
    mode->setEditorValue(state["centerValue"], 0);

    return dlg(qApp->tr("Flip"), {dir, mode}, "flip", [&]{
        state["dir"] = _params.dir = dir->selection();
        state["centerMode"] = _params.centerMode = mode->selection();
        state["centerValue"] = _params.centerValue = mode->editor()->value();
    });
}

//------------------------------------------------------------------------------
//                              FlipRawModifier
//------------------------------------------------------------------------------

bool UpendModifier::configure()
{
    auto dir = new AxisOption;
    auto val = new ValueOption(qApp->tr("Value"));

    State state("upend");
    dir->setSelection(state["dir"]);
    val->setValue(state["value"]);

    return dlg(qApp->tr("Upend"), {dir, val}, "flip", [&]{
        state["dir"] = _params.dir = dir->selection();
        state["value"] = _params.value = val->value();
    });
}

//------------------------------------------------------------------------------
//                              ScaleModifier
//------------------------------------------------------------------------------

bool ScaleModifier::configure()
{
    auto dir = new AxisOption;
    auto center = new RadioOptions<Scale::CenterMode>(qApp->tr("Scale Center"),
        {{ Scale::CENTER_ZERO, qApp->tr("Zero") },
         { Scale::CENTER_MAX, qApp->tr("Maximum") },
         { Scale::CENTER_MIN, qApp->tr("Minimum") },
         { Scale::CENTER_AVG, qApp->tr("Average") },
         { Scale::CENTER_MID, qApp->tr("Median") },
         { Scale::CENTER_VAL, qApp->tr("Arbitrary value") }});
    center->makeEditor(Scale::CENTER_VAL);
    auto factor = new ValueOption(qApp->tr("Factor"));

    State state("scale");
    dir->setSelection(state["dir"]);
    center->setSelection(state["centerMode"]);
    center->setEditorValue(state["centerValue"], 0);
    factor->setValue(state["scaleFactor"], 1);

    return dlg(qApp->tr("Scale"), {dir, center, factor}, "scale", [&]{
        state["dir"] = _params.dir = dir->selection();
        state["centerMode"] = _params.centerMode = center->selection();
        state["centerValue"] = _params.centerValue = center->editor()->value();
        state["scaleFactor"] = _params.scaleFactor = factor->value();
    });
}

//------------------------------------------------------------------------------
//                              NormalizeModifier
//------------------------------------------------------------------------------

bool NormalizeModifier::configure()
{
    auto dir = new AxisOption;
    auto mode = new RadioOptions<Normalize::Mode>(qApp->tr("Factor"),
        {{ Normalize::MODE_MAX, qApp->tr("Maximum") },
         { Normalize::MODE_VAL, qApp->tr("Arbitrary value") }});
    mode->makeEditor(Normalize::MODE_VAL);

    State state("normalize");
    dir->setSelection(state["dir"]);
    mode->setSelection(state["mode"]);
    mode->setEditorValue(state["value"]);

    return dlg(qApp->tr("Normalize"), {dir, mode}, "normalize", [&]{
        state["dir"] = _params.dir = dir->selection();
        state["mode"] = _params.mode = mode->selection();
        state["value"] = _params.value = mode->editor()->value();
    });
}

//------------------------------------------------------------------------------
//                              InvertModifier
//------------------------------------------------------------------------------

bool InvertModifier::configure()
{
    auto dir = new AxisOption;
    auto val = new ValueOption(qApp->tr("Value"));

    State state("invert");
    dir->setSelection(state["dir"]);
    val->setValue(state["value"].toDouble(1));

    return dlg(qApp->tr("Invert"), {dir, val}, "invert", [&]{
        state["dir"] = _params.dir = dir->selection();
        state["value"] = _params.value = val->value();
    });
}

//------------------------------------------------------------------------------
//                              DecimateModifier
//------------------------------------------------------------------------------

bool DecimateModifier::configure()
{
    auto intv = new IntervalOption(qApp->tr("Interval"));

    State state("decimate");
    intv->setPoints(state["points"], 2);
    intv->setStep(state["step"], 1);
    intv->setUseStep(state["useStep"]);

    return dlg(qApp->tr("Decimate"), {intv}, "decimate", [&]{
        state["points"] = _params.points = intv->points();
        state["step"] = _params.step = intv->step();
        state["useStep"] = _params.useStep = intv->useStep();
    });
}

//------------------------------------------------------------------------------
//                              AverageModifier
//------------------------------------------------------------------------------

bool AverageModifier::configure()
{
    auto intv = new IntervalOption(qApp->tr("Interval"));
    auto pos = new RadioOptions<Average::PointPos>(qApp->tr("Point Position"), {
        { Average::POS_BEG, qApp->tr("Begin of interval") },
        { Average::POS_MID, qApp->tr("Middle of interval") },
        { Average::POS_END, qApp->tr("End of interval") },
    });

    State state("average");
    intv->setPoints(state["points"], 2);
    intv->setStep(state["step"], 1);
    intv->setUseStep(state["useStep"]);
    pos->setSelection(state["pointPos"], Average::POS_MID);

    return dlg(qApp->tr("Average"), {intv, pos}, "average", [&]{
        state["points"] = _params.points = intv->points();
        state["step"] = _params.step = intv->step();
        state["useStep"] = _params.useStep = intv->useStep();
        state["pointPos"] = _params.pointPos = pos->selection();
    });
}

//------------------------------------------------------------------------------
//                            FitLimitsModifier
//------------------------------------------------------------------------------

bool FitLimitsModifier::configure()
{
    auto dir = new AxisOption;

    auto lims = new QGroupBox(qApp->tr("New Limits"));
    auto layout = new QFormLayout(lims);
    auto begLabel = new QLabel(qApp->tr("Left"));
    auto endLabel = new QLabel(qApp->tr("Right"));
    auto beg = makeEditor();
    auto end = makeEditor();
    layout->addRow(begLabel, beg);
    layout->addRow(endLabel, end);

    dir->connect(dir->button(DIR_X), &QRadioButton::toggled, dir, [begLabel, endLabel](bool alongX){
        begLabel->setText(alongX ? qApp->tr("Left") : qApp->tr("Bottom"));
        endLabel->setText(alongX ? qApp->tr("Right") : qApp->tr("Top"));
    });

    State state("fitLimits");
    dir->setSelection(state["dir"], DIR_X);
    beg->setValue(state["beg"].toDouble(0));
    end->setValue(state["end"].toDouble(1));

    return dlg(qApp->tr("Fit Limits"), {dir, lims}, "fit_limits", [&]{
        state["dir"] = _params.dir = dir->selection();
        state["beg"] = _params.beg = beg->value();
        state["end"] = _params.end = end->value();
    });
}
