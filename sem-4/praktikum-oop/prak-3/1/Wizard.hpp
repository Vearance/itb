#pragma once
#include "Maiar.hpp"

class Wizard : private Maiar {
private:
    string staffName;

public:
    Wizard(const string& name, int hp, int power, const string& staffName);

    void describe() const override;
    void cast() const;
};
