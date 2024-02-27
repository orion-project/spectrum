#include "Operations.h"

#include "CsvConfigDialog.h"
#include "CustomPrefs.h"
#include "OpenFileDlg.h"
#include "core/Graph.h"
#include "core/DataSources.h"
#include "core/Modifiers.h"
#include "widgets/RangeEditor.h"

#include "helpers/OriDialogs.h"
#include "helpers/OriLayouts.h"
#include "widgets/OriPopupMessage.h"

#include <QApplication>
#include <QDebug>
#include <QGroupBox>

#define SELECTED_GRAPH \
    auto graph = getSelectedGraph(); \
    if (!graph) { \
        Ori::Gui::PopupMessage::warning(qApp->tr("Please select a graph")); \
        return; \
    }

Operations::Operations(QObject *parent) : QObject(parent)
{
}

void Operations::addFromFile()
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

void Operations::addFromCsvFile()
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

void Operations::addFromClipboardCsv()
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

void Operations::addFromClipboard()
{
    addGraph(new ClipboardDataSource);
}

void Operations::addRandomSample()
{
    auto root = CustomDataHelpers::loadDataSourceStates();
    auto rnd = root["random_sample"].toObject();

    RandomSampleParams params;
    params.rangeY.min = rnd["min"].toDouble(0);
    params.rangeY.max = rnd["max"].toDouble(100);
    params.rangeX.start = rnd["start"].toDouble(0);
    params.rangeX.stop = rnd["stop"].toDouble(99);
    params.rangeX.step = rnd["step"].toDouble(1);
    params.rangeX.points = rnd["points"].toInt(100);
    params.rangeX.useStep = rnd["useStep"].toBool(false);

    auto editorX = new RangeEditor;
    editorX->setRange(params.rangeX);

    auto editorY = new MinMaxEditor;
    editorY->setValue(params.rangeY);

    auto editor = Ori::Layouts::LayoutV({
        Ori::Layouts::LayoutV({editorX}).makeGroupBox("X"),
        Ori::Layouts::LayoutV({editorY}).makeGroupBox("Y"),
    }).setMargin(0).makeWidgetAuto();

    if (Ori::Dlg::Dialog(editor.get(), false)
        .withTitle(tr("Random Sample Params"))
        .withContentToButtonsSpacingFactor(3)
        .withVerification([editorX]{ return editorX->range().verify(); })
        .exec())
    {
        params.rangeX = editorX->range();
        params.rangeY = editorY->value();

        addGraph(new RandomSampleDataSource(params));

        rnd["min"] = params.rangeY.min;
        rnd["max"] = params.rangeY.max;
        rnd["start"] = params.rangeX.start;
        rnd["stop"] = params.rangeX.stop;
        rnd["step"] = params.rangeX.step;
        rnd["points"] = params.rangeX.points;
        rnd["useStep"] = params.rangeX.useStep;
        root["random_sample"] = rnd;
        CustomDataHelpers::saveDataSourceStates(root);
    }
}

void Operations::modifyOffset()
{
    modifyGraph(new OffsetModifier);
}

void Operations::modifyScale()
{
    modifyGraph(new ScaleModifier);
}

void Operations::addGraph(DataSource* dataSource, DoConfig doConfig, DoLoad doLoad)
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

void Operations::modifyGraph(Modifier* mod)
{
    // TODO: modify several selected graphs
    SELECTED_GRAPH

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

void Operations::graphRefresh()
{
    // TODO: refresh several selected graphs
    SELECTED_GRAPH

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

void Operations::graphReopen()
{
    SELECTED_GRAPH

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
