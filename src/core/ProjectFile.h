#ifndef PROJECT_FILE_H
#define PROJECT_FILE_H

#include <QVector>
#include <QHash>
#include <QJsonObject>

class Project;
class Diagram;
class Graph;

class ProjectFile
{
public:
    struct StorableData
    {
        QString fileName;
        Project *project;
        QVector<Diagram*> diagrams;
        QHash<const void*, QJsonObject> formats;
    };
    
    static QString saveProject(const StorableData &data);
    static QString loadProject(const QString &fileName, Project *project);
    
private:
    static QJsonObject writeProject(const Project *p);
    static QJsonObject writeDiagram(const Diagram *d);
    static QJsonObject writeGraph(const Graph *g);
    static QByteArray writeGraphData(const Graph *g);
    
    static QString readProject(const QJsonObject &obj, Project *p);
    static QString readDiagram(const QJsonObject &obj, Diagram *d);
    static QString readGraph(const QJsonObject &obj, Graph *g);
    static QString readGraphData(const QByteArray &data, Graph *g);
};

#endif // PROJECT_FILE_H
