#include "Operations.h"

#include "core/Graph.h"
#include "core/DataSources.h"
#include "core/Modificators.h"
#include "helpers/OriDialogs.h"

#include <QApplication>
#include <QFileDialog>
#include <QDebug>

Operations::Operations(QObject *parent) : QObject(parent)
{
}

void Operations::makeFromFile() const
{
    QString fileName = QFileDialog::getOpenFileName(qApp->activeWindow());
    if (fileName.isEmpty()) return;

    // TODO choose a data source respecting to the file extension and selected filter

    addGraph(new TextFileDataSource(fileName));
}

void Operations::makeFromClipboard() const
{
    addGraph(new ClipboardDataSource);
}

void Operations::makeRandomSample() const
{
    addGraph(new RandomSampleDataSource);
}

void Operations::makeRandomSampleParams() const
{
    // TODO
}

void Operations::modifyOffset() const
{
    modifyGraph(new OffsetModificator);
}

void Operations::modifyScale() const
{
    modifyGraph(new ScaleModificator);
}

void Operations::addGraph(DataSource* dataSource) const
{
    auto graph = new Graph(dataSource);
    auto res = graph->refreshData();
    if (!res.isEmpty())
    {
        Ori::Dlg::error(res);
        delete graph;
        return;
    }
    emit graphCreated(graph);
}

void Operations::modifyGraph(Modificator* mod) const
{
    auto graph = getSelectedGraph();
    if (!graph)
    {
        Ori::Dlg::info("Select some graph");
        return;
    }
    if (!mod->configure())
    {
        delete mod;
        return;
    }
    auto res = graph->modify(mod);
    if (!res.isEmpty())
    {
        Ori::Dlg::error(res);
        delete mod;
        return;
    }
    emit graphUpdated(graph);
}

void Operations::graphRefresh() const
{
    auto graph = getSelectedGraph();
    if (!graph)
    {
        Ori::Dlg::info("Select some graph");
        return;
    }
    auto res = graph->canRefreshData();
    if (!res.isEmpty())
    {
        Ori::Dlg::info(res);
        return;
    }
    res = graph->refreshData();
    if (!res.isEmpty())
    {
        Ori::Dlg::error(res);
        return;
    }
    emit graphUpdated(graph);
}
