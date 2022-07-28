#include "Graph.h"

#include "DataSources.h"
#include "Modifiers.h"

Graph::Graph(DataSource* dataSource): _dataSource(dataSource)
{
    _data = _dataSource->initialData();
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

QString Graph::refreshData()
{
    auto res = _dataSource->getData();
    if (!res.ok())
        return res.error();

    if (_autoTitle)
        _title = _dataSource->makeTitle();

    _data = res.result();

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
