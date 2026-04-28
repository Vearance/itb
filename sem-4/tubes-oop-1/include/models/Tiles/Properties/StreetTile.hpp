#pragma once
#include "ColorGroup.hpp"
#include "PropertyTile.hpp"
#include <vector>

/// @brief Real estate street tile with color groups, houses, hotels, and festival effects.
class StreetTile : public PropertyTile {
private:
    ColorGroup colorGroup;

    bool monopolyOwned = false;
    int propertyLevel = 0; ///< 0–4 = houses, 5 = hotel
    int housePrice;
    int hotelPrice;

public:
    StreetTile(int id, const std::string& code, const std::string& name, int price,
               ColorGroup colorGroup, int mortgageValue, int housePrice, int hotelPrice,
               const std::vector<int>& rentByLevel);
    ~StreetTile() override;

    void landedOn(IGameContext& ctx) override;

    // ── Color group ────────────────────────────────────────────────────────────
    ColorGroup getColorGroup() const;

    // ── Building management ────────────────────────────────────────────────────
    int getPropertyLevel() const;
    void setPropertyLevel(int level);
    bool buildHouse();
    bool buildHotel();
    int getHousePrice() const;
    int getHotelPrice() const;

    /// @brief Remove one building level (used by DemolitionCard and liquidation).
    bool demolish() override;

    // ── Monopoly flag ──────────────────────────────────────────────────────────
    bool isMonopolyOwned() const;
    void setMonopolyOwned(bool value);

    // ── Rent ───────────────────────────────────────────────────────────────────
    int calculateRent(const Player& player) const override;
};
