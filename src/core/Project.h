#ifndef PROJECT_H
#define PROJECT_H

#include <QColor>
#include <QHash>
#include <QIcon>
#include <QObject>

class Diagram;

class Project : public QObject
{
    Q_OBJECT

public:
    Project(QObject *parent);
    
    void newDiagram();
    void deleteDiagram(const QString &id);
    
    bool modified() const { return _modified; }
    void markModified(const QString &reason);
    
    Diagram* diagram(const QString &id);
    
    QString getSaveFileName();
    
private:
    QHash<QString, Diagram*> _items;
    int _nextDiagramIndex = 0;
    int _nextDiagramColorIndex = 0;
    bool _modified = false;

    QColor nextDiagramColor();
};


class Diagram : public QObject
{
    Q_OBJECT

public:
    Project* project() const { return _prj; }
    QString id() const { return _id; }
    const QString& title() const { return _title; }
    const QColor& color() const { return _color; }
    const QIcon& icon();

    void markModified(const QString &reason);
    
    void doRename();
    void doDelete();

private:
    Diagram(Project *project);

    Project *_prj;
    QString _id;
    QString _title;
    QColor _color;
    QIcon _icon;

    friend class PlotWindow;
    friend class Project;
};

#endif // PROJECT_H
