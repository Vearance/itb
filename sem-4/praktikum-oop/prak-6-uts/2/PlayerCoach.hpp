#pragma once
#include "Player.hpp"
#include "Staff.hpp"

class PlayerCoach : public Player, public Staff
{
private:
    // TODO: deklarasikan atribut kelas (yearsAsPlayer, isCurrentlyPlaying)
    int yearsAsPlayer;
    bool isCurrentlyPlaying;

public:
    // TODO: deklarasikan constructor, methods work(), calculateRating(), getSpecialty(), calculateWage(), getProfile(), dan destructor
    PlayerCoach(std::string name, int age, std::string contractEnd, std::string position, int stamina, double rating, std::string license, int yearsAsPlayer, bool isCurrentlyPlaying);

    void work() const override;
    double calculateRating() const override;
    std::string getSpecialty() const override;
    double calculateWage() const override;
    std::string getProfile() const override;

    ~PlayerCoach() override;
};
