#ifndef MODIFICATORS_H
#define MODIFICATORS_H

#include "BaseTypes.h"
#include "GraphMath.h"

class Modificator
{
public:
    virtual ~Modificator();
    virtual GraphResult modify(const GraphPoints& data) const = 0;
    virtual bool configure() { return true; }
};


class OffsetModificator : public Modificator
{
public:
    GraphResult modify(const GraphPoints& data) const override;
    bool configure() override;
private:
    GraphMath::Offset _params;
};


class ScaleModificator : public Modificator
{
public:
    GraphResult modify(const GraphPoints& data) const override;
    bool configure() override;
private:
    GraphMath::Scale _params;
};

#endif // MODIFICATORS_H
