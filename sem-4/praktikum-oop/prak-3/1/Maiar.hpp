#pragma once
#include "Creature.hpp"

class Maiar : protected Creature {
protected:
    int power;

    Maiar(const string& name, int hp, int power);

public:
    void describe() const override;
    int getPower() const;
};
