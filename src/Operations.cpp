#include "Operations.h"
#include "core/Graph.h"
#include "helpers/OriDialogs.h"
#include "funcs/FuncPlotTextFile.h"
#include "funcs/FuncPlotTextClipboard.h"
#include "funcs/FuncRandomSample.h"

#include <QApplication>
#include <QFileDialog>

Operations::Operations(QObject *parent) : QObject(parent)
{
}

void Operations::makeGraphFromFile() const
{
    QString fileName = QFileDialog::getOpenFileName(qApp->activeWindow());
    if (fileName.isEmpty()) return;

    // TODO choose a function respecting to file extension and selected filter

    FuncPlotTextFile f(fileName);
    processFunc(&f);
}

void Operations::makeGraphFromClipboard() const
{
    FuncPlotTextClipboard f;
    processFunc(&f);
}

void Operations::makeRandomSample() const
{
    FuncRandomSample f;
    processFunc(&f);
}

void Operations::makeRandomSampleParams() const
{
    FuncRandomSampleWithParams f;
    processFunc(&f);
}

void Operations::processFunc(FuncBase* func) const
{
    if (func->configurable() && !func->configure())
        return;

    bool ok = func->process();
    if (!ok)
        return Ori::Dlg::error(func->error());

    auto g = new Graph;
    g->setTitle(func->title());
    g->setData(func->data());

    emit graphCreated(g);
}
