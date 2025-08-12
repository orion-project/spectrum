#ifndef MODIFIERS_H
#define MODIFIERS_H

#include "BaseTypes.h"
#include "GraphMath.h"

#include <QJsonObject>

class Modifier
{
public:
    virtual ~Modifier() {}
    virtual GraphResult modify(const GraphPoints& data) const = 0;
    virtual bool configure() { return true; }
    virtual void save(QJsonObject &root) const = 0;
};

template <typename TParams>
class ModifierBase : public Modifier
{
public:
    GraphResult modify(const GraphPoints &data) const override {
        return GraphResult::ok(_params.calc(data));
    }
protected:
    TParams _params;
};

#define MODIFIER(mod) \
    class mod##Modifier : public ModifierBase<GraphMath::mod> { \
    public: \
        bool configure() override; \
        void save(QJsonObject &root) const override { \
            root["type"] = QString::fromLatin1(#mod); \
            _params.save(root); \
        } \
    };

MODIFIER(Offset)
MODIFIER(Reflect)
MODIFIER(Flip)
MODIFIER(Scale)
MODIFIER(Normalize)
MODIFIER(Invert)
MODIFIER(Decimate)
MODIFIER(Average)
MODIFIER(MavgSimple)
MODIFIER(MavgCumul)
MODIFIER(MavgExp)
MODIFIER(FitLimits)
MODIFIER(Despike)
MODIFIER(Derivative)

#endif // MODIFIERS_H
