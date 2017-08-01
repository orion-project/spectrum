#include "FuncBase.h"
#include "helpers/OriLayouts.h"
#include "helpers/OriDialogs.h"
#include "tools/OriSettings.h"

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMetaProperty>

void saveFuncParams(const QString& title, QObject* params)
{
    Ori::Settings s;
    s.beginGroup(title);

    auto meta = params->metaObject();
    for (int i = 0; i < meta->propertyCount(); i++)
    {
        QMetaProperty prop = meta->property(i);
        QString propName(prop.name());
        if (propName == "objectName") continue;

        s.setValue(propName, params->property(prop.name()));
    }
}

void loadFuncParams(const QString& title, QObject* params)
{
    Ori::Settings s;
    s.beginGroup(title);

    auto meta = params->metaObject();
    for (int i = 0; i < meta->propertyCount(); i++)
    {
        QMetaProperty prop = meta->property(i);
        QString propName(prop.name());
        if (propName == "objectName") continue;

        if (s.settings()->contains(propName))
            params->setProperty(prop.name(), s.value(propName));
        else
            prop.reset(params);
    }
}

//-----------------------------------------------------------------------------

//class FuncParamsEditorDlg : public QDialog
//{
//    Q_OBJECT

//public:
//    FuncParamsEditorDlg(FuncParams* params, FuncParamsEditorBase* paramsEditor) : QDialog(qApp->activeWindow())
//    {
//        _params = params;
//        _paramsEditor = paramsEditor;


//        Ori::Layouts::LayoutV({paramsEditor, buttons}).useFor(this);

//        _paramsEditor->populate(_params);
//    }

//private slots:
//    void apply()
//    {
//        QString res = _paramsEditor->verify();
//        if (!res.isEmpty())
//            return Ori::Dlg::warning(res);

//        _paramsEditor->collect(_params);

//        accept();
//    }

//private:
//    FuncParams* _params;
//    FuncParamsEditorBase* _paramsEditor;
//};


bool askFuncParams(QObject *params, FuncParamsEditorBase* paramsEditor)
{
    auto paramsName = params->objectName();
//    if (paramsName.isEmpty())
//        paramsName = params->defaultName();
    loadFuncParams(paramsName, params);
    paramsEditor->populate(params);

    QDialog dlg(qApp->activeWindow());

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    qApp->connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    qApp->connect(buttons, &QDialogButtonBox::accepted, [params, paramsEditor, &dlg](){
        auto res = paramsEditor->verify();
        if (res.isEmpty()) dlg.accept();
        Ori::Dlg::warning(res);
    });

    Ori::Layouts::LayoutV({paramsEditor, buttons}).useFor(&dlg);

    bool ok = dlg.exec() == QDialog::Accepted;
    if (ok)
    {
        paramsEditor->collect(params);
        saveFuncParams(paramsName, params);
    }
    return ok;
}

//-----------------------------------------------------------------------------

//bool ConfigFuncBase::configure()
//{
//    if (_params) delete _params;
//    _params = makeParams();
//    return askFuncParams(_params, makeParamsEditor());
//}
