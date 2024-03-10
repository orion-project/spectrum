#include "Graph.h"

#include "core/DataSources.h"
#include "core/Modifiers.h"

#include <QUuid>

//------------------------------------------------------------------------------
//                                  Graph
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

//------------------------------------------------------------------------------
//                                 PlotObj
//------------------------------------------------------------------------------

PlotObj::PlotObj()
{
    _id = QUuid::createUuid().toString(QUuid::Id128);
}
