#ifndef MODIFICATORS_H
#define MODIFICATORS_H

#include "BaseTypes.h"

class Modificator
{
public:
    virtual ~Modificator();
    virtual GraphResult modify(const GraphPoints& data) const = 0;
};


class OffsetModificator : public Modificator
{
public:
    GraphResult modify(const GraphPoints& data) const override;
};

#endif // MODIFICATORS_H
