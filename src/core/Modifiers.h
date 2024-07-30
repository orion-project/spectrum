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

class OffsetModifier : public ModifierBase<GraphMath::Offset>
{
public:
    bool configure() override;
};

class FlipModifier : public ModifierBase<GraphMath::Flip>
{
public:
    bool configure() override;
};

class FlipRawModifier : public ModifierBase<GraphMath::FlipRaw>
{
public:
    bool configure() override;
};

class ScaleModifier : public ModifierBase<GraphMath::Scale>
{
public:
    bool configure() override;
};

class NormalizeModifier : public ModifierBase<GraphMath::Normalize>
{
public:
    bool configure() override;
};

class InvertModifier : public ModifierBase<GraphMath::Invert>
{
public:
    bool configure() override;
};

#endif // MODIFIERS_H
