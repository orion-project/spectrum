#ifndef MODIFIERS_H
#define MODIFIERS_H

#include "BaseTypes.h"
#include "GraphMath.h"

class Modifier
{
public:
    virtual ~Modifier();
    virtual GraphResult modify(const GraphPoints& data) const = 0;
    virtual bool configure() { return true; }
};


class OffsetModifier : public Modifier
{
public:
    GraphResult modify(const GraphPoints& data) const override;
    bool configure() override;
private:
    GraphMath::Offset _params;
};


class ScaleModifier : public Modifier
{
public:
    GraphResult modify(const GraphPoints& data) const override;
    bool configure() override;
private:
    GraphMath::Scale _params;
};

#endif // MODIFIERS_H
