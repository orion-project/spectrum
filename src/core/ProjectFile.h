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
        QVector<const Diagram*> diagrams;
        QHash<const void*, QJsonObject> formats;
    };
    
    static QString saveProject(const StorableData &data);
    
private:
    static QJsonObject writeProject(const Project *p);
    static QJsonObject writeDiagram(const Diagram *d);
    static QJsonObject writeGraph(const Graph *g);
    static QByteArray writeGraphData(const Graph *g);
};

#endif // PROJECT_FILE_H
