#ifndef FUNCBASE_H
#define FUNCBASE_H

#include "qcpl_types.h"

#include <QWidget>

class FuncBase
{
public:
    virtual ~FuncBase() {}

    virtual bool configurable() const { return false; }
    virtual bool configure() { return false; }
    virtual bool process() = 0;

    const QString& title() const { return _title; }
    const QString& error() const { return _error; }
    const QCPL::GraphData& data() const { return _data; }

protected:
    QString _title;
    QString _error;
    QCPL::GraphData _data;
};

//-----------------------------------------------------------------------------

class FuncParams: public QObject
{
    Q_OBJECT
public:
    QString defaultName() const { return metaObject()->className(); }
    FuncParams() : QObject() {}
    ~FuncParams() {}
};

#define FUNC_PARAMS(class_name)\
class class_name : public FuncParams {\
public:

#define FUNC_PARAM(param_type, param_name, default_value)\
    Q_PROPERTY(param_type _##param_name##_ MEMBER param_name RESET setDefault_##param_name)\
    param_type param_name;\
    void setDefault_##param_name() { param_name = default_value; }


#define FUNC_PARAMS_END };

//-----------------------------------------------------------------------------

class FuncParamsEditorBase : public QWidget
{
public:
    virtual QString verify() { return QString(); }
    virtual void populate(QObject* params) = 0;
    virtual void collect(QObject* params) = 0;
};

bool askFuncParams(QObject* params, FuncParamsEditorBase* paramsEditor);

//-----------------------------------------------------------------------------

class ConfigFuncBase : public FuncBase
{
public:
    ~ConfigFuncBase() { if (_params) delete _params; }

    bool configurable() const override { return true; }

    bool configure() override
    {
        if (_params) delete _params;
        _params = makeParams();
        return askFuncParams(_params, makeParamsEditor());
    }

protected:
    FuncParams* _params = nullptr;

    virtual FuncParams* makeParams() = 0;
    virtual FuncParamsEditorBase* makeParamsEditor() = 0;
};

//-----------------------------------------------------------------------------

#endif // FUNCBASE_H
