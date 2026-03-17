#include "Maiar.hpp"
#include <iostream>

Maiar::Maiar(const string& name, int hp, int power): Creature(name, hp), power(power) {}

void Maiar::describe() const {
    std::cout << "Maiar [" << name << "] | HP: " << hp << " | Power: " << power << "\n";
}

int Maiar::getPower() const {
    return power;
}
