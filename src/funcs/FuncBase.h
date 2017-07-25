#ifndef FUNCBASE_H
#define FUNCBASE_H

#include "qcpl_types.h"

class FuncBase
{
public:
    virtual ~FuncBase() {}

    virtual bool process() = 0;

    const QString& title() const { return _title; }
    const QString& error() const { return _error; }
    const QCPL::GraphData& data() const { return _data; }

protected:
    QString _title;
    QString _error;
    QCPL::GraphData _data;
};

#endif // FUNCBASE_H
