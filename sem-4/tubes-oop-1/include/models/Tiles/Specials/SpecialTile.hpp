#pragma once
#include "Tile.hpp"

/**
 * @brief Intermediate base for corner/special tiles (Go, Jail, Free Parking, Go To Jail).
 * Same pattern as ActionTile: landedOn() delegates to executeAction().
 */
class SpecialTile : public Tile {
public:
    SpecialTile(int id, const std::string& code, const std::string& name);
    ~SpecialTile() override;

    void landedOn(IGameContext& ctx) override;

protected:
    virtual void executeAction(IGameContext& ctx) = 0;
};
