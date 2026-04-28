#include "MoveCard.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"
#include <string>

using namespace std;

MoveCard::MoveCard(const string& name, const string& description, int steps)
    : SkillCard(name, description), steps(steps) {}

MoveCard::~MoveCard() {}

int MoveCard::getSteps() const {
    return steps;
}

void MoveCard::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    ctx.printMessage("MoveCard: maju " + to_string(steps) + " petak.\n");
    ctx.movePlayerBy(player, steps);
}
