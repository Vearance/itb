#pragma once
#include "Creature.hpp"

class Elf : public Creature {
protected:
    int grace;

public:
    Elf(const string& name, int hp, int grace);

    void describe() const override;
    int getGrace() const;
};
