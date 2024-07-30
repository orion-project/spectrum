#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "core/OriArg.h"

#include <QObject>

#include <functional>

class Graph;
class DataSource;
class Modifier;

namespace Ori {
class MruFileList;
}


class Operations : public QObject
{
    Q_OBJECT

public:
    explicit Operations(QObject *parent = nullptr);

    std::function<Graph*()> getSelectedGraph;

    using DoConfig = Ori::Argument<bool, struct DoConfigTag>;
    using DoLoad = Ori::Argument<bool, struct DoLoadTag>;

    Ori::MruFileList* mruPlotFormats() { return _mruPlotFormats; }

public slots:
    void addFromFile();
    void addFromCsvFile();
    void addFromClipboard();
    void addFromClipboardCsv();
    void addRandomSample();
    void modifyOffset();
    void modifyFlip();
    void modifyFlipRaw();
    void modifyScale();
    void modifyNormalize();
    void modifyInvert();
    void graphRefresh();
    void graphReopen();

signals:
    void graphCreated(Graph* g);
    void graphUpdated(Graph* g);

private:
    Ori::MruFileList *_mruPlotFormats;

    void addGraph(DataSource* dataSource, DoConfig doConfig = DoConfig(true), DoLoad doLoad = DoLoad(true));
    void modifyGraph(Modifier *mod);
};

#endif // OPERATIONS_H
