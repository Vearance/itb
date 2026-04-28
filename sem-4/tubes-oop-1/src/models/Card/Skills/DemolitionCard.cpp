#include "DemolitionCard.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"
#include "PropertyTile.hpp"

using namespace std;

DemolitionCard::DemolitionCard(const string& name, const string& description)
    : SkillCard(name, description) {}

DemolitionCard::~DemolitionCard() {}

void DemolitionCard::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    PropertyTile* target = ctx.promptSelectOpponentProperty(player);
    if (target) {
        ctx.printMessage("DemolitionCard: " + target->getName() + " dihancurkan!\n");
        ctx.destroyPropertyToBank(*target);
    }
}
