#include "StreetTile.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"
#include <algorithm>

using namespace std;

StreetTile::StreetTile(int id, const string& code, const string& name, int price,
                       ColorGroup colorGroup, int mortgageValue, int housePrice, int hotelPrice,
                       const vector<int>& rentByLevel)
    : PropertyTile(id, code, name, PropertyType::STREET, price, mortgageValue, rentByLevel),
      colorGroup(colorGroup),
      monopolyOwned(false),
      propertyLevel(0),
      housePrice(housePrice),
      hotelPrice(hotelPrice) {}

StreetTile::~StreetTile() {}

void StreetTile::landedOn(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    if (isBankOwned()) {
        int effectivePrice = getPrice();
        if (player.hasActiveDiscount()) {
            effectivePrice -= effectivePrice * player.getDiscountPercentage() / 100;
        }
        bool wantsToBuy = ctx.promptBuyProperty(player, *this);
        if (wantsToBuy && player.getMoney() >= effectivePrice) {
            ctx.chargeVoluntary(player, effectivePrice);
            if (player.getStatus() != PlayerStatus::BANKRUPT) {
                ctx.grantProperty(player, *this);
            }
        } else {
            ctx.initiateAuction(*this);
        }
        return;
    }
    PropertyTile::landedOn(ctx);
}

ColorGroup StreetTile::getColorGroup() const {
    return colorGroup;
}

int StreetTile::getPropertyLevel() const {
    return propertyLevel;
}

void StreetTile::setPropertyLevel(int level) {
    propertyLevel = clamp(level, 0, 5);
}

bool StreetTile::buildHouse() {
    if (propertyLevel < 0 || propertyLevel >= 4) return false;
    ++propertyLevel;
    return true;
}

bool StreetTile::buildHotel() {
    if (propertyLevel != 4) return false;
    propertyLevel = 5;
    return true;
}

bool StreetTile::demolish() {
    if (propertyLevel <= 0) return false;
    --propertyLevel;
    return true;
}

int StreetTile::getHousePrice() const {
    return housePrice;
}

int StreetTile::getHotelPrice() const {
    return hotelPrice;
}

bool StreetTile::isMonopolyOwned() const {
    return monopolyOwned;
}

void StreetTile::setMonopolyOwned(bool value) {
    monopolyOwned = value;
}

int StreetTile::calculateRent(const Player& player) const {
    if (!canCollectRentFrom(player)) return 0;

    int rent = getRentByIndex(propertyLevel);
    if (propertyLevel == 0 && monopolyOwned) {
        rent *= 2;
    }
    if (hasFestival()) {
        rent *= getFestivalMultiplier();
    }
    return rent;
}
