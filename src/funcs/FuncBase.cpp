#include "FuncBase.h"
#include "tools/OriSettings.h"

#include <QObject>
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
