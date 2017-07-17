#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QWidget>

namespace QCPL {
class Plot;
}

class PlotWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PlotWindow(QWidget *parent = nullptr);

    QSize sizeHint() const override { return QSize(600, 400); }

private:
    QCPL::Plot* _plot;
};

#endif // PLOTWINDOW_H
