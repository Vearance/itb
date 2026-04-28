#include "UtilityTile.hpp"
#include "IGameContext.hpp"
#include "GameConfig.hpp"
#include "Player.hpp"
#include <algorithm>

using namespace std;

UtilityTile::UtilityTile(int id, const string& code, const string& name, int mortgageValue)
    : PropertyTile(id, code, name, PropertyType::UTILITY, 0, mortgageValue, {}) {}

UtilityTile::~UtilityTile() {}

void UtilityTile::landedOn(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    if (isBankOwned()) {
        ctx.grantProperty(player, *this);
        return;
    }
    if (!canCollectRentFrom(player)) return;
    int rent = calculateRent(ctx);
    ctx.transferMoney(player, *getOwner(), rent);
}

int UtilityTile::calculateRent(IGameContext& ctx) const {
    const Player* owner = getOwner();
    if (!owner) return 0;

    int ownedCount = 0;
    for (const PropertyTile* prop : owner->getProperties()) {
        if (prop->getType() == PropertyType::UTILITY) {
            ++ownedCount;
        }
    }

    const auto& multipliers = ctx.getConfig().utilityMultipliers;
    int index = clamp(ownedCount - 1, 0, static_cast<int>(multipliers.size()) - 1);
    int rent = ctx.getLastDiceTotal() * multipliers[index];
    if (hasFestival()) {
        rent *= getFestivalMultiplier();
    }
    return rent;
}

int UtilityTile::calculateRent(const Player& /*player*/) const {
    return 0;
}
