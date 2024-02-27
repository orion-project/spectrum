#include "RangeEditor.h"

#include "helpers/OriLayouts.h"
#include "widgets/OriValueEdit.h"

#include <QApplication>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>

using Ori::Widgets::ValueEdit;
using namespace Ori::Layouts;

//------------------------------------------------------------------------------
//                                 RangeEditor
//------------------------------------------------------------------------------

RangeEditor::RangeEditor()
{
    edStart = new ValueEdit;
    edStop = new ValueEdit;

    rbPoints = new QRadioButton(qApp->tr("Number of points", "Variable range editor"));

    sePoints = new QSpinBox;
    sePoints->setAlignment(Qt::AlignRight);
    sePoints->setMinimum(2);
    sePoints->setMaximum(999999999);
    connect(sePoints, SIGNAL(valueChanged(int)), rbPoints, SLOT(click()));

    rbStep = new QRadioButton(qApp->tr("With step", "Variable range editor"));

    edStep = new ValueEdit;
    connect(edStep, SIGNAL(valueChanged(double)), rbStep, SLOT(click()));

    auto grid = new QGridLayout;
    grid->addWidget(edStep, 0, 1); // before radio-buttons meaning tab-order
    grid->addWidget(sePoints, 1, 1);
    grid->addWidget(rbStep, 0, 0);
    grid->addWidget(rbPoints, 1, 0);

    LayoutV({
        LayoutH({
            new QLabel(tr("From")), edStart,
            SpaceH(2),
            new QLabel(tr("To")), edStop,
        }),
        LayoutH({
            Stretch(),
            grid
        }),
    }).setMargin(0).useFor(this);
}

void RangeEditor::setRange(const PlottingRange& range)
{
    edStart->setValue(range.start);
    edStop->setValue(range.stop);
    edStep->setValue(range.step);
    sePoints->setValue(range.points);
    if (range.useStep)
        rbStep->setChecked(true);
    else
        rbPoints->setChecked(true);
}

PlottingRange RangeEditor::range() const
{
    PlottingRange range;
    range.start = edStart->value();
    range.stop = edStop->value();
    range.step = edStep->value();
    range.points = sePoints->value();
    range.useStep = rbStep->isChecked();
    return range;
}

//------------------------------------------------------------------------------
//                                 MinMaxEditor
//------------------------------------------------------------------------------

MinMaxEditor::MinMaxEditor()
{
    edMin = new ValueEdit;
    edMax = new ValueEdit;

    LayoutH({
        new QLabel(tr("Min")), edMin,
        SpaceH(2),
        new QLabel(tr("Max")), edMax,
    }).setMargin(0).useFor(this);
}

MinMax MinMaxEditor::value() const
{
    double min = edMin->value();
    double max = edMax->value();
    if (min > max) qSwap(min, max);
    return MinMax { .min = min, .max = max };
}

void MinMaxEditor::setValue(const MinMax& v)
{
    edMin->setValue(v.min);
    edMax->setValue(v.max);
}
