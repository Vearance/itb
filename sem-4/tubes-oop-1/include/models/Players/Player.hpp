#pragma once
#include "ColorGroup.hpp"
#include <string>
#include <vector>

// Forward declarations — avoid pulling heavy headers into every translation unit
class PropertyTile;
class SkillCard;

enum class PlayerStatus {
    ACTIVE,
    BANKRUPT,
    JAILED
};

/// @brief Represents a player in the game.
class Player {
private:
    int id;
    std::string username;
    int money;
    PlayerStatus status;
    int position;
    int jailTurns;
    bool isComputer;

    std::vector<SkillCard*> hand;
    std::vector<PropertyTile*> properties;

    bool hasUsedSkillThisTurn;
    bool hasRolledDice;
    int consecutiveDoubles;

    // Active card-effect state
    int discountPercentage; ///< 0 = no active discount
    int discountTurnsLeft;
    int shieldTurnsLeft;   ///< 0 = not shielded
    bool getOutOfJailCard; ///< true if player has a "Bebas dari Penjara" card

public:
    Player(int id, const std::string& username);
    ~Player();

    // ── Operator overloads ─────────────────────────────────────────────────────
    Player& operator+=(int amount);
    Player& operator-=(int amount);
    /// @brief Compare by cash on hand.
    bool operator<(const Player& other) const;
    bool operator>(const Player& other) const;

    // ── Basic getters ──────────────────────────────────────────────────────────
    int getId() const;
    const std::string& getUsername() const;
    int getMoney() const;
    int getPosition() const;
    PlayerStatus getStatus() const;
    int getJailTurns() const;
    int getConsecutiveDoubles() const;
    const std::vector<SkillCard*>& getHand() const;
    const std::vector<PropertyTile*>& getProperties() const;

    bool getIsComputer() const;
    void setIsComputer(bool value);

    // ── Wealth helpers ─────────────────────────────────────────────────────────
    /// @brief Cash on hand only.
    int getLiquidWealth() const;
    /// @brief Cash + full buy-price of all owned properties + building costs.
    int getTotalWealth() const;
    /// @brief Sum of full buy-prices of all owned properties (including mortgaged).
    int getPropertyValue() const;

    // ── Setters ────────────────────────────────────────────────────────────────
    void setPosition(int pos);
    void setStatus(PlayerStatus newStatus);
    void setJailTurns(int turns);
    void incrementJailTurns();
    void resetJailTurns();

    // ── Turn-state flags ───────────────────────────────────────────────────────
    bool getHasUsedSkillThisTurn() const;
    void setUsedSkillThisTurn(bool value);
    bool getHasRolledDice() const;
    void setRolledDice(bool value);
    void resetTurnFlags();

    // ── Jail helpers ───────────────────────────────────────────────────────────
    bool isInJail() const;
    void goToJail();
    void releaseFromJail();

    // ── Property management ────────────────────────────────────────────────────
    void addProperty(PropertyTile* property);
    void removeProperty(PropertyTile* property);
    bool hasMonopoly(ColorGroup group) const;
    /// @brief Returns groups where the player owns all tiles.
    std::vector<ColorGroup> getMonopolyGroups() const;

    // ── Skill card management ──────────────────────────────────────────────────
    /// @brief Add card to hand. Throws CardLimitException if hand already has 3.
    void addCard(SkillCard* card);
    void removeCard(SkillCard* card);
    bool hasCard(SkillCard* card) const;

    // ── Active card effects ────────────────────────────────────────────────────
    void applyDiscount(int percentage, int turns);
    void tickDiscount();
    bool hasActiveDiscount() const;
    int getDiscountPercentage() const;

    void activateShield(int turns);
    void tickShield();
    bool isShielded() const;

    // ── "Bebas dari Penjara" card ───────────────────────────────────────────────
    bool hasJailFreeCard() const;
    void setJailFreeCard(bool value);
    void useJailFreeCard();
};
