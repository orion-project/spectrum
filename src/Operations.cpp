#include "Operations.h"
#include "core/Graph.h"
#include "helpers/OriDialogs.h"
#include "funcs/FuncMakeFromClipboard.h"
#include "funcs/FuncRandomSample.h"

Operations::Operations(QObject *parent) : QObject(parent)
{
}

void Operations::makeRandomSample() const
{
    FuncRandomSample f;
    processFunc(&f);
}

void Operations::makeGraphFromClipboard() const
{
    FuncMakeFromClipboard f;
    processFunc(&f);
}

void Operations::processFunc(FuncBase* func) const
{
    bool ok = func->process();
    if (!ok)
        return Ori::Dlg::error(func->error());

    auto g = new Graph;
    g->setTitle(func->title());
    g->setData(func->data());

    emit graphCreated(g);
}
