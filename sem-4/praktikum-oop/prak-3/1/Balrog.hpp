#pragma once
#include "Maiar.hpp"

class Balrog : public Maiar {
private:
    string whipName;

public:
    Balrog(const string& name, int hp, int power, const string& whipName);

    void describe() const override;
    void rage() const;

    using Maiar::getPower;
};
