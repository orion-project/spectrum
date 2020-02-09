#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <QObject>
#include <functional>

class Graph;
class DataSource;
class Modificator;

class Operations : public QObject
{
    Q_OBJECT

public:
    explicit Operations(QObject *parent = nullptr);

    void addFromFile() const;
    void addFromClipboard() const;
    void addRandomSample() const;
    void modifyOffset() const;
    void modifyScale() const;
    void graphRefresh() const;
    void graphReopen() const;

    std::function<Graph*()> getSelectedGraph;

signals:
    void graphCreated(Graph* g) const;
    void graphUpdated(Graph* g) const;

private:
    void addGraph(DataSource* dataSource) const;
    void modifyGraph(Modificator* mod) const;
};

#endif // OPERATIONS_H
