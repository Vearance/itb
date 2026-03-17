#include "Balrog.hpp"
#include <iostream>

Balrog::Balrog(const string& name, int hp, int power, const string& whipName)
    : Maiar(name, hp, power), whipName(whipName) {}

void Balrog::describe() const {
    std::cout << "Balrog [" << name << "] | HP: " << hp << " | Power: " << power
              << " | Whip: " << whipName << "\n";
}

void Balrog::rage() const {
    std::cout << name << " cracks " << whipName << " with " << power << " power!\n";
}
