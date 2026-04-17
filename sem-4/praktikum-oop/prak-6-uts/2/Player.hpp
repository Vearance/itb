#pragma once
#include "ClubMember.hpp"
#include <string>

using namespace std;

class Player : virtual public ClubMember
{
protected:
    // TODO: deklarasikan atribut kelas (position, stamina, rating)
    string position;
    int stamina;
    double rating;

public:
    // TODO: deklarasikan constructor, method calculateWage() dan pure virtual calculateRating() serta destructor
    Player(string name, int age, string contractEnd, string position, int stamina, double rating);
    virtual ~Player();
    double calculateWage() const override;
    virtual double calculateRating() const = 0;
};
