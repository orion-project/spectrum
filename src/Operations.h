#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "core/OriArg.h"

#include <QObject>

#include <functional>

class Graph;
class DataSource;
class Modifier;
class Project;

namespace Ori {
class MruFileList;
class Settings;
}


class Operations : public QObject
{
    Q_OBJECT

public:
    explicit Operations(Project *project, QObject *parent = nullptr);
    
    void restoreState(Ori::Settings &s);

    std::function<Graph*()> getSelectedGraph;
    std::function<QHash<const void*, QJsonObject>()> getFormats;

    using DoConfig = Ori::Argument<bool, struct DoConfigTag>;
    using DoLoad = Ori::Argument<bool, struct DoLoadTag>;

    Ori::MruFileList* mruPlotFormats() { return _mruPlotFormats; }
    Ori::MruFileList* mruProjects() { return _mruProjects; }
    
    bool canClose();

    void openPrjFile(const QString& fileName);
    bool savePrjFile(const QString& fileName);

public slots:
    void prjNew();
    void prjOpen();
    bool prjSave();
    bool prjSaveAs();
    void addFromFile();
    void addFromCsvFile();
    void addFromClipboard();
    void addFromClipboardCsv();
    void addRandomSample();
    void modifyOffset();
    void modifyReflect();
    void modifyFlip();
    void modifyScale();
    void modifyNormalize();
    void modifyInvert();
    void modifyDecimate();
    void modifyAverage();
    void modifyMavgSimple();
    void modifyMavgCumul();
    void modifyMavgExp();
    void modifyFitLimits();
    void modifyDespike();
    void modifyDerivative();
    void graphRefresh();
    void graphReopen();

signals:
    void graphCreated(Graph* g);
    
private:
    Project *_project;
    Ori::MruFileList *_mruProjects;
    Ori::MruFileList *_mruPlotFormats;
    
    void addGraph(DataSource* dataSource, DoConfig doConfig = DoConfig(true), DoLoad doLoad = DoLoad(true));
    void modifyGraph(Modifier *mod);
};

#endif // OPERATIONS_H
