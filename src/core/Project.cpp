#include "Project.h"

#include "BaseTypes.h"
#include "DataSources.h"
#include "Modifiers.h"

#include "qcpl_colors.h"

#include <QDebug>
#include <QFileDialog>
#include <QPainter>
#include <QUuid>

//------------------------------------------------------------------------------
//                                 Project
//------------------------------------------------------------------------------

Project::Project(QObject *parent) : QObject(parent)
{
}

void Project::newDiagram()
{
    auto dia = new Diagram(this);
    dia->_id = QUuid::createUuid().toString(QUuid::Id128);
    dia->_title = tr("Diagram %1").arg(++_nextDiagramIndex);
    dia->_color = nextDiagramColor();
    _diagrams.insert(dia->id(), dia);
    qDebug() << "Project::newDiagram" << dia->id();
    BusEvent::DiagramAdded::send({{"id", dia->id()}});
    markModified("Project::newDiagram");
}

void Project::deleteDiagram(const QString &id)
{
    auto dia = _diagrams.value(id);
    if (!dia) {
        qWarning() << "Project::deleteDiagram: diagram not found" << id;
        return;
    }
    _diagrams.remove(id);
    delete dia;
    BusEvent::DiagramDeleted::send({{"id", id}});
    markModified("Project::deleteDiagram " + id);
}

Diagram* Project::diagram(const QString &id)
{
    return _diagrams.value(id);
}

QVector<Diagram*> Project::diagrams() const
{
    QVector<Diagram*> res;
    for (auto it = _diagrams.cbegin(); it != _diagrams.cend(); it++)
        res << it.value();
    return res;
}

Graph* Project::graph(const QString &id)
{
    for (auto it = _diagrams.cbegin(); it != _diagrams.cend(); it++)
        if (auto g = it.value()->graph(id); g)
            return g;
    return nullptr;
}

QColor Project::nextDiagramColor()
{
    if (_nextDiagramColorIndex == QCPL::defaultColorSet().size())
        _nextDiagramColorIndex = 0;
    return QCPL::defaultColorSet().at(_nextDiagramColorIndex++);
}

void Project::markModified(const QString &reason)
{
    _modified = true;
    qDebug() << "Project::modified" << reason;
    BusEvent::ProjectModified::send();
}

void Project::markUnmodified(const QString &reason)
{
    _modified = false;
    qDebug() << "Project::unmodified" << reason;
    BusEvent::ProjectUnmodified::send();
}

void Project::updateGraph(Graph *graph)
{
    BusEvent::GraphUpdated::send({{"id", graph->id()}});
    markModified("Project::updateGraph");
}

//------------------------------------------------------------------------------
//                                 Diagram
//------------------------------------------------------------------------------

Diagram::Diagram(Project *project): QObject(project), _prj(project)
{
}

static QIcon makeGraphIcon(QColor color)
{
    int H, S, L;
    color.getHsl(&H, &S, &L);
    QColor backColor = QColor::fromHsl(H, int(float(S)*0.8f), int(float(L)*1.2f));
    QColor borderColor = QColor::fromHsl(H, int(float(S)*0.5f), L);

    QPixmap px(16, 16);
    px.fill(Qt::transparent);

    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing, true);

    QPen borderPen(borderColor);
    borderPen.setWidthF(1.5);
    p.setPen(borderPen);

    p.setBrush(backColor);
    p.drawEllipse(px.rect().adjusted(1, 1, -1, -1));

    // TODO draw gradient gloss

    return QIcon(px);
}

const QIcon& Diagram::icon()
{
    if (_icon.isNull())
        _icon = makeGraphIcon(_color);
    return _icon;
}

void Diagram::markModified(const QString &reason)
{
    _prj->markModified(reason);
}

Graph* Diagram::graph(const QString &id)
{
    return _graphs.value(id);
}

void Diagram::addGraph(Graph *g)
{
    _graphs.insert(g->id(), g);
    BusEvent::GraphAdded::send({{"id", g->id()}});
    markModified("Diagram::addGraph");
}

void Diagram::deleteGraphs(const QVector<Graph*> &graphs)
{
    for (auto g : std::as_const(graphs)) {
        QString id = g->id();
        BusEvent::GraphDeleting::send({{"id", id}});
        _graphs.remove(id);
        delete g;
        BusEvent::GraphDeleted::send({{"id", id}});
    }
    markModified("Diagram::deleteGraphs");
}

//------------------------------------------------------------------------------
//                                 Graph
//------------------------------------------------------------------------------

Graph::Graph(DataSource* dataSource): _dataSource(dataSource)
{
    _id = QUuid::createUuid().toString(QUuid::Id128);
    _data = _dataSource->data();
    _title = _dataSource->makeTitle();
}

Graph::~Graph()
{
    delete _dataSource;
    qDeleteAll(_modifiers);
}

void Graph::setColor(const QColor& color)
{
    _color = color;
    _icon = QIcon();
}

const QIcon& Graph::icon()
{
    if (_icon.isNull())
        _icon = makeGraphIcon(_color);
    return _icon;
}

QString Graph::canRefreshData() const
{
    return _dataSource->canRefresh();
}

QString Graph::refreshData(bool reread)
{
    if (reread)
    {
        auto res = _dataSource->read();
        if (!res.ok())
            return res.error();

        _data = res.result();
    }
    else
        _data = _dataSource->data();

    if (_autoTitle)
        _title = _dataSource->makeTitle();

    foreach (auto mod, _modifiers)
    {
        auto res = mod->modify(_data);
        if (!res.ok())
            return res.error();

        _data = res.result();
    }

    return QString();
}

QString Graph::modify(Modifier* mod)
{
    auto res = mod->modify(_data);
    if (!res.ok())
        return res.error();

    _modifiers.append(mod);

    _data = res.result();
    return QString();
}
