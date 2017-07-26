#ifndef FUNCBASE_H
#define FUNCBASE_H

#include "qcpl_types.h"

#include <QObject>

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

#define FUNC_PARAMS(class_name)\
    class class_name : public QObject {\
    Q_OBJECT

#define FUNC_PARAM(param_type, param_name)\
    public:\
    Q_PROPERTY(param_type param_name ## _ MEMBER param_name)\
    param_type param_name;

#define FUNC_PARAMS_END };

void saveFuncParams(const QString &title, QObject* params);

#endif // FUNCBASE_H
