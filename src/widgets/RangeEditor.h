#ifndef RANGE_EDITOR_H
#define RANGE_EDITOR_H

#include <QWidget>

#include "../core/BaseTypes.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QRadioButton;
class QSpinBox;
QT_END_NAMESPACE

namespace Ori::Widgets {
class ValueEdit;
}

class RangeEditor : public QWidget
{
    Q_OBJECT

public:
    RangeEditor();

    PlottingRange range() const;
    void setRange(const PlottingRange& range);

private:
    QSpinBox *sePoints;
    QRadioButton *rbStep, *rbPoints;
    Ori::Widgets::ValueEdit *edStart, *edStop, *edStep;
};

class MinMaxEditor : public QWidget
{
    Q_OBJECT

public:
    MinMaxEditor();

    MinMax value() const;
    void setValue(const MinMax& v);

private:
    Ori::Widgets::ValueEdit *edMin, *edMax;
};

#endif // RANGE_EDITOR_H
