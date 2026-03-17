#include "Wizard.hpp"
#include <iostream>

Wizard::Wizard(const string& name, int hp, int power, const string& staffName)
    : Maiar(name, hp, power), staffName(staffName) {}

void Wizard::describe() const {
    std::cout << "Wizard [" << name << "] | HP: " << hp << " | Staff: " << staffName << "\n";
}

void Wizard::cast() const {
    std::cout << name << " channels " << power << " power through " << staffName << "!\n";
}
