#include "Operations.h"

#include "CsvConfigDialog.h"
#include "CustomPrefs.h"
#include "OpenFileDlg.h"
#include "core/Graph.h"
#include "core/DataSources.h"
#include "core/Modifiers.h"

#include "helpers/OriDialogs.h"

#include <QApplication>
#include <QDebug>

Operations::Operations(QObject *parent) : QObject(parent)
{
}

void Operations::addFromFile() const
{
    auto states = CustomDataHelpers::loadDataSourceStates();
    auto state = states["file"].toObject();

    OpenFileDlg dlg;
    if (dlg.open(&state))
    {
        states["file"] = state;
        CustomDataHelpers::saveDataSourceStates(states);

        foreach (const QString& file, dlg.files)
            addGraph(new TextFileDataSource(file), DoConfig(false));
    }
}

void Operations::addFromCsvFile() const
{
    auto res = CsvConfigDialog::openFile();
    if (res.dataSources.isEmpty())
    {
        if (!res.report.isEmpty())
            Ori::Dlg::error(res.report.join("\n"));
        return;
    }
    if (!res.report.isEmpty())
        Ori::Dlg::warning(res.report.join("\n"));
    // CsvConfigDialog loads all graphs in optimized way
    // no need to load each graph separately
    foreach (auto dataSource, res.dataSources)
        addGraph(dataSource, DoConfig(false), DoLoad(false));
}

void Operations::addFromClipboardCsv() const
{
    auto res = CsvConfigDialog::openClipboard();
    if (res.dataSources.isEmpty())
    {
        if (!res.report.isEmpty())
            Ori::Dlg::error(res.report.join("\n"));
        return;
    }
    if (!res.report.isEmpty())
        Ori::Dlg::warning(res.report.join("\n"));
    // CsvConfigDialog loads all graphs in optimized way
    // no need to load each graph separately
    foreach (auto dataSource, res.dataSources)
        addGraph(dataSource, DoConfig(false), DoLoad(false));
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
    modifyGraph(new OffsetModifier);
}

void Operations::modifyScale() const
{
    modifyGraph(new ScaleModifier);
}

void Operations::addGraph(DataSource* dataSource, DoConfig doConfig, DoLoad doLoad) const
{
    if (doConfig.value)
    {
        auto cr = dataSource->configure();
        if (cr.ok)
        {
            if (!cr.error.isEmpty())
                Ori::Dlg::error(cr.error);
            delete dataSource;
            return;
        }
    }

    auto graph = new Graph(dataSource);

    auto res = graph->refreshData(doLoad.value);
    if (!res.isEmpty())
    {
        Ori::Dlg::error(res);
        delete graph;
        return;
    }

    emit graphCreated(graph);
}

void Operations::modifyGraph(Modifier* mod) const
{
    // TODO: modify several selected graphs
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
    // TODO: refresh several selected graphs
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

    // TODO: check if graph has no data anymore
    // (e.g. file was deleted or its content changed unexpectedly)
    // and add an ability to cancel and keep old data
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

    auto res = dataSource->canRefresh();
    if (!res.isEmpty())
    {
        Ori::Dlg::info(res);
        return;
    }

    auto cr = dataSource->configure();
    if (!cr.ok)
    {
        if (!cr.error.isEmpty())
            Ori::Dlg::error(cr.error);
        return;
    }

    // TODO: check if new config issues no data (e.g. wrong file selected)
    // and add an ability to rollback the config

    res = graph->refreshData();
    if (!res.isEmpty())
    {
        Ori::Dlg::error(res);
        return;
    }
    emit graphUpdated(graph);
}
