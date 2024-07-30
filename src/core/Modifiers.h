#ifndef MODIFIERS_H
#define MODIFIERS_H

#include "BaseTypes.h"
#include "GraphMath.h"

class Modifier
{
public:
    virtual ~Modifier() {}
    virtual GraphResult modify(const GraphPoints& data) const = 0;
    virtual bool configure() { return true; }
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
    };

MODIFIER(Offset)
MODIFIER(Flip)
MODIFIER(Upend)
MODIFIER(Scale)
MODIFIER(Normalize)
MODIFIER(Invert)
MODIFIER(Decimate)
MODIFIER(Average)
MODIFIER(FitLimits)

#endif // MODIFIERS_H
