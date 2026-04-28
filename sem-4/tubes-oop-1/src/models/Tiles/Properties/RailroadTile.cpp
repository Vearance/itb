#include "RailroadTile.hpp"
#include "IGameContext.hpp"
#include "GameConfig.hpp"
#include "Player.hpp"
#include <algorithm>

using namespace std;

RailroadTile::RailroadTile(int id, const string& code, const string& name, int mortgageValue)
    : PropertyTile(id, code, name, PropertyType::RAILROAD, 0, mortgageValue, {}) {}

RailroadTile::~RailroadTile() {}

void RailroadTile::landedOn(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    if (isBankOwned()) {
        ctx.grantProperty(player, *this);
        return;
    }
    if (!canCollectRentFrom(player)) return;
    int rent = calculateRent(ctx);
    ctx.transferMoney(player, *getOwner(), rent);
}

int RailroadTile::calculateRent(IGameContext& ctx) const {
    const Player* owner = getOwner();
    if (!owner) return 0;

    int ownedCount = 0;
    for (const PropertyTile* prop : owner->getProperties()) {
        if (prop->getType() == PropertyType::RAILROAD) {
            ++ownedCount;
        }
    }

    const auto& rentTable = ctx.getConfig().railroadRentTable;
    int index = clamp(ownedCount - 1, 0, static_cast<int>(rentTable.size()) - 1);
    int rent = rentTable[index];
    if (hasFestival()) {
        rent *= getFestivalMultiplier();
    }
    return rent;
}

int RailroadTile::calculateRent(const Player& /*player*/) const {
    return 0;
}
