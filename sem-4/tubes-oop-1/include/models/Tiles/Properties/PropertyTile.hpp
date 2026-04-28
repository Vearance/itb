#pragma once
#include "Tile.hpp"
#include <string>
#include <vector>

// Forward declaration — avoids circular include with Player.hpp
class Player;
class IGameContext;

enum class PropertyStatus {
    BANK,
    OWNED,
    MORTGAGED
};

enum class PropertyType {
    STREET,
    RAILROAD,
    UTILITY
};

/// @brief Abstract base for all purchasable property tiles.
class PropertyTile : public Tile {
private:
    Player* owner;
    PropertyStatus status;
    PropertyType type;
    int price;
    int mortgageValue;
    std::vector<int> rentTable;

    int festivalMultiplier = 1; ///< 1 = inactive, up to 8x (max 3 doublings)
    int festivalDur = 0; ///< Turns remaining for festival; 0 = inactive

public:
    PropertyTile(int id, const std::string& code, const std::string& name, PropertyType type,
                 int price, int mortgageValue, const std::vector<int>& rentTable = {});
    ~PropertyTile() override;

    /// @brief Default landing: triggers rent payment or auto-acquire (Railroad/Utility).
    void landedOn(IGameContext& ctx) override;

    // ── Ownership ──────────────────────────────────────────────────────────────
    Player* getOwner() const;
    void setOwner(Player* newOwner);
    void releaseToBank();

    PropertyStatus getStatus() const;
    void setStatus(PropertyStatus newStatus);

    // ── Data accessors ─────────────────────────────────────────────────────────
    int getPrice() const;
    PropertyType getType() const;
    int getMortgageValue() const;
    const std::vector<int>& getRentTable() const;
    int getRentByIndex(int index) const;

    // ── Status helpers ─────────────────────────────────────────────────────────
    bool isBankOwned() const;
    bool isMortgaged() const;
    bool canCollectRentFrom(const Player& player) const;

    void mortgage();
    void unmortgage();

    /// @brief Allow a building to be demolished (overridden by StreetTile).
    virtual bool demolish() { return false; }

    // ── Festival ───────────────────────────────────────────────────────────────
    /// @brief Each call doubles rent (up to max 8x) and resets duration to 3 turns.
    void activateFestival();
    void tickFestival();
    bool hasFestival() const;
    int getFestivalMultiplier() const;
    int getFestivalDur() const;
    /// @brief Directly restore festival state (used by save/load).
    void setFestivalState(int multiplier, int duration);

    // ── Pure virtual ───────────────────────────────────────────────────────────
    /// @brief Calculate the rent due when `player` lands on this tile.
    virtual int calculateRent(const Player& player) const = 0;
};
