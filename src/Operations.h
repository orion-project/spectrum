#ifndef OPERATIONS_H
#define OPERATIONS_H

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

public slots:
    void addFromFile() const;
    void addFromCsvFile() const;
    void addFromClipboard() const;
    void addFromClipboardCsv() const;
    void addRandomSample() const;
    void modifyOffset() const;
    void modifyScale() const;
    void graphRefresh() const;
    void graphReopen() const;

signals:
    void graphCreated(Graph* g) const;
    void graphUpdated(Graph* g) const;

private:
    void addGraph(DataSource* dataSource, bool configured = false) const;
    void modifyGraph(Modifier *mod) const;
};

#endif // OPERATIONS_H
