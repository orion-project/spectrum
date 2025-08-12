#ifndef GRAPH_H
#define GRAPH_H

#include <QIcon>

#include "BaseTypes.h"


class DataSource;
class Modifier;

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
};

#endif // GRAPH_H
