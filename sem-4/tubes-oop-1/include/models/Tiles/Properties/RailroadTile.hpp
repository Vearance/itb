#pragma once
#include "PropertyTile.hpp"

/// @brief Railroad (station) tile. Acquired automatically; rent scales with total railroads owned.
class RailroadTile : public PropertyTile {
public:
    RailroadTile(int id, const std::string& code, const std::string& name, int mortgageValue);
    ~RailroadTile() override;

    void landedOn(IGameContext& ctx) override;

    /// @brief Rent = railroadRentTable[ownedCount - 1] from GameConfig.
    int calculateRent(const Player& player) const override;

    /// @brief Context-aware overload used by landedOn to access config data.
    int calculateRent(IGameContext& ctx) const;
};
