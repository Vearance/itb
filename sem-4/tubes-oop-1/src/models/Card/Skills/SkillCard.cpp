#include "SkillCard.hpp"
#include "Player.hpp"

using namespace std;

SkillCard::SkillCard(const string& name, const string& description) : Card(name, description) {}

SkillCard::~SkillCard() {}

bool SkillCard::usable(const Player& player) const {
    return !player.getHasUsedSkillThisTurn() && !player.getHasRolledDice();
}
