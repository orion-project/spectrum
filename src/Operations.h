#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "core/OriArg.h"

#include <QObject>

#include <functional>

class Graph;
class DataSource;
class Modifier;

class Operations : public QObject
{
    Q_OBJECT

public:
    explicit Operations(QObject *parent = nullptr);

    std::function<Graph*()> getSelectedGraph;

    using DoConfig = Ori::Argument<bool, struct DoConfigTag>;
    using DoLoad = Ori::Argument<bool, struct DoLoadTag>;

public slots:
    void addFromFile();
    void addFromCsvFile();
    void addFromClipboard();
    void addFromClipboardCsv();
    void addRandomSample();
    void modifyOffset();
    void modifyScale();
    void graphRefresh();
    void graphReopen();

signals:
    void graphCreated(Graph* g);
    void graphUpdated(Graph* g);

private:
    void addGraph(DataSource* dataSource, DoConfig doConfig = DoConfig(true), DoLoad doLoad = DoLoad(true));
    void modifyGraph(Modifier *mod);
};

#endif // OPERATIONS_H
