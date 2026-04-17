#include "Player.hpp"
#include "Formatter.hpp"

Player::Player(string name, int age, string contractEnd, string pos, int sta, double rat)
    : ClubMember(name, age, contractEnd), position(pos), stamina(sta), rating(rat)
{
    // TODO: constructor

}

double Player::calculateWage() const {
    // TODO: return wage player dengan rumus rating * 10000.0
    return rating*10000.0;
}

Player::~Player() {
    // TODO: log destruction menggunakan Formatter::log
    Formatter::log("~Player", name, "training log freed.");
}
