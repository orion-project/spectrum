#ifndef PROJECT_H
#define PROJECT_H

#include "BaseTypes.h"

#include <QColor>
#include <QHash>
#include <QIcon>

class DataSource;
class Diagram;
class Graph;
class Modifier;

class Project : public QObject
{
    Q_OBJECT

public:
    Project(QObject *parent);
    
    QString fileName() const { return _fileName; }
    void setFileName(const QString &v) { _fileName = v; }
    
    Diagram* diagram(const QString &id);
    QVector<Diagram*> diagrams() const;
    void newDiagram();
    void deleteDiagram(const QString &id);
    
    bool modified() const { return _modified; }
    void markModified(const QString &reason);
    void markUnmodified(const QString &reason);
    
    Graph* graph(const QString &id);
    void updateGraph(Graph *graph);
    
private:
    QString _fileName;
    QHash<QString, Diagram*> _diagrams;
    int _nextDiagramIndex = 0;
    int _nextDiagramColorIndex = 0;
    bool _modified = false;

    QColor nextDiagramColor();
    
    friend class ProjectFile;
};


class Diagram : public QObject
{
    Q_OBJECT

public:
    Project* project() const { return _prj; }
    QString id() const { return _id; }
    const QString& title() const { return _title; }
    void setTitle(const QString &s) { _title = s; }
    const QColor& color() const { return _color; }
    const QIcon& icon();

    void markModified(const QString &reason);
    
    Graph* graph(const QString &id);
    void addGraph(Graph *g);
    void deleteGraphs(const QVector<Graph*> &graphs);
    
private:
    Diagram(Project *project);

    Project *_prj;
    QString _id;
    QString _title;
    QColor _color;
    QIcon _icon;
    QHash<QString, Graph*> _graphs;

    friend class Project;
    friend class ProjectFile;
};


class Graph
{
public:
    Graph(DataSource* dataSource);
    ~Graph();

    QString id() const { return _id; }

    const QString& title() const { return _title; }
    void setTitle(const QString& title) { _title = title; _autoTitle = false; }

    const QColor& color() const { return _color; }
    void setColor(const QColor& color) { _color = color; }

    const QIcon& icon() const { return _icon; }
    void setIcon(const QIcon& icon) { _icon = icon; }

    DataSource* dataSource() const { return _dataSource; }
    const GraphPoints& data() const { return _data; }
    int pointsCount() const { return _data.xs.size(); }

    QString canRefreshData() const;
    QString refreshData(bool reread = true);

    /// The graph takes ownership on the modificator.
    QString modify(Modifier* mod);

private:
    QString _id;
    bool _autoTitle = true;
    DataSource* _dataSource;
    QList<Modifier*> _modifiers;
    GraphPoints _data;
    QString _title;
    QIcon _icon;
    QColor _color;
    
    friend class ProjectFile;
};

#endif // PROJECT_H
