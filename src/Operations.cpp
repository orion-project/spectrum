#include "Operations.h"

#include "CsvConfigDialog.h"
#include "core/Graph.h"
#include "core/DataSources.h"
#include "core/Modificators.h"
#include "helpers/OriDialogs.h"

#include <QApplication>
#include <QDebug>

Operations::Operations(QObject *parent) : QObject(parent)
{
}

void Operations::addFromFile() const
{
    // TODO: open several files in the same time
    addGraph(new TextFileDataSource());
}

void Operations::addFromCsvFile() const
{
    auto dataSources = CsvConfigDialog::openFile();
    if (!dataSources.ok())
    {
        Ori::Dlg::error(dataSources.error());
        return;
    }
    // TODO: optimize repainting
    for (auto dataSource : dataSources.result())
        emit graphCreated(new Graph(dataSource));
}

void Operations::addFromClipboardCsv() const
{
    auto dataSources = CsvConfigDialog::openClipboard();
    if (!dataSources.ok())
    {
        Ori::Dlg::error(dataSources.error());
        return;
    }
    // TODO: optimize repainting
    for (auto dataSource : dataSources.result())
        emit graphCreated(new Graph(dataSource));
}

void Operations::addFromClipboard() const
{
    addGraph(new ClipboardDataSource);
}

void Operations::addRandomSample() const
{
    addGraph(new RandomSampleDataSource);
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
    if (!dataSource->configure())
    {
        delete dataSource;
        return;
    }
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
        Ori::Dlg::info(qApp->tr("Please select a graph"));
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
        Ori::Dlg::info(qApp->tr("Please select a graph"));
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

void Operations::graphReopen() const
{
    auto graph = getSelectedGraph();
    if (!graph)
    {
        Ori::Dlg::info(qApp->tr("Please select a graph"));
        return;
    }

    auto dataSource = graph->dataSource();
    if (dynamic_cast<TextFileDataSource*>(dataSource))
    {
        if (!dataSource->configure())
            return;
    }
    else if (dynamic_cast<CsvFileDataSource*>(dataSource))
    {
        if (!CsvConfigDialog::openExisted(dynamic_cast<CsvFileDataSource*>(dataSource)))
            return;
    }
    else
    {
        Ori::Dlg::info(qApp->tr("Graph's data source is not a file"));
        return;
    }
    auto res = graph->refreshData();
    if (!res.isEmpty())
    {
        Ori::Dlg::error(res);
        return;
    }
    emit graphUpdated(graph);
}
