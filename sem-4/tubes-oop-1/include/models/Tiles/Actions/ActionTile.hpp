#pragma once
#include "Tile.hpp"

/**
 * @brief Intermediate base for tiles that trigger a specific action when landed on.
 *
 * Subclasses implement executeAction(). landedOn() is the public entry point
 * (called by the game engine) and simply delegates to executeAction().
 */
class ActionTile : public Tile {
public:
    ActionTile(int id, const std::string& code, const std::string& name);
    ~ActionTile() override;

    void landedOn(IGameContext& ctx) override;

protected:
    virtual void executeAction(IGameContext& ctx) = 0;
};
