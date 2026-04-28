#include "ChanceCard.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"

using namespace std;

ChanceCard::ChanceCard(const string& name, const string& description, ChanceEffect effect)
    : Card(name, description), effect(effect) {}

ChanceCard::~ChanceCard() {}

ChanceEffect ChanceCard::getEffect() const {
    return effect;
}

void ChanceCard::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    switch (effect) {
        case ChanceEffect::GO_TO_NEAREST_STATION:
            ctx.movePlayerToNearest(player, "RAILROAD");
            break;
        case ChanceEffect::MOVE_BACK_3:
            ctx.movePlayerBy(player, -3);
            break;
        case ChanceEffect::GO_TO_JAIL:
            ctx.sendPlayerToJail(player);
            break;
        case ChanceEffect::GET_OUT_OF_JAIL:
            player.setJailFreeCard(true);
            break;
    }
}
