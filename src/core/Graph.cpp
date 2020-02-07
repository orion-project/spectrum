#include "Graph.h"

#include "DataSources.h"
#include "Modificators.h"

Graph::Graph(DataSource* dataSource): _dataSource(dataSource)
{
}

Graph::~Graph()
{
    delete _dataSource;
    qDeleteAll(_modificators);
}

QString Graph::canRefreshData() const
{
    return _dataSource->canRefresh();
}

QString Graph::refreshData()
{
    auto res = _dataSource->getData();
    if (!res.ok())
        return res.error();

    if (_autoTitle)
        _title = _dataSource->makeTitle();

    _xs = res.result().xs;
    _ys = res.result().ys;

    for (auto mod : _modificators)
    {
        auto res = mod->modify({_xs, _ys});
        if (!res.ok())
            return res.error();

        _xs = res.result().xs;
        _ys = res.result().ys;
    }

    return QString();
}

QString Graph::modify(Modificator* mod)
{
    auto res = mod->modify({_xs, _ys});
    if (!res.ok())
        return res.error();

    _modificators.append(mod);

    _xs = res.result().xs;
    _ys = res.result().ys;
    return QString();
}
