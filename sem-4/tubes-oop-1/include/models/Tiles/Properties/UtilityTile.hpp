#pragma once
#include "PropertyTile.hpp"

/// @brief Utility tile (PLN/PAM). Acquired automatically; rent = dice total × multiplier.
class UtilityTile : public PropertyTile {
public:
    UtilityTile(int id, const std::string& code, const std::string& name, int mortgageValue);
    ~UtilityTile() override;

    void landedOn(IGameContext& ctx) override;

    /// @brief Rent = ctx.getLastDiceTotal() × utilityMultipliers[ownedCount - 1].
    int calculateRent(const Player& player) const override;

    /// @brief Context-aware overload used by landedOn to access dice total and config.
    int calculateRent(IGameContext& ctx) const;
};
