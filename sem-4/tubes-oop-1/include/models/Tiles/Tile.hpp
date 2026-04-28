#pragma once
#include <string>

// Forward declaration — avoids pulling in Player/Game headers here
class IGameContext;

/// @brief Abstract base class for all board tiles.
class Tile {
private:
    int id;
    std::string code;
    std::string name;

public:
    Tile(int id, const std::string& code, const std::string& name);
    virtual ~Tile();

    int getId() const;
    const std::string& getCode() const;
    const std::string& getName() const;

    /**
     * @brief Called when a player lands on this tile.
     * @param ctx Live game context — use to move players, charge money, trigger events.
     */
    virtual void landedOn(IGameContext& ctx) = 0;
};
