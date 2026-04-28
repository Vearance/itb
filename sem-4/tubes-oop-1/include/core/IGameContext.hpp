#pragma once
#include <string>
#include <vector>

// Forward declarations — keeps this interface header dependency-free
class Player;
class Board;
class PropertyTile;
class Logger;
class GameConfig;

/**
 * @brief Interface that represents the game's runtime context.
 *
 * Tiles and Cards depend on this interface instead of the concrete Game class,
 * which decouples game-logic objects from the engine and prevents a God Class.
 * GameEngine implements this interface and delegates UI calls to GameView.
 *
 * Analogous to a Java "GameManager" interface used in event-driven game loops.
 */
class IGameContext {
public:
    virtual ~IGameContext() = default;

    // ── State queries ──────────────────────────────────────────────────────────
    virtual Player& getActivePlayer() = 0;
    virtual const std::vector<Player*> getAllActivePlayers() const = 0;
    virtual Board& getBoard() = 0;
    virtual const GameConfig& getConfig() const = 0;
    virtual int getCurrentTurn() const = 0;
    virtual Logger& getLogger() = 0;

    /** Total of the last dice roll — needed by UtilityTile rent calculation. */
    virtual int getLastDiceTotal() const = 0;

    /** Print a message through the UI layer. */
    virtual void printMessage(const std::string& message) = 0;

    // ── Player movement ────────────────────────────────────────────────────────
    /** Move player forward by steps; grants GO salary if lap is completed. */
    virtual void movePlayerBy(Player& player, int steps) = 0;

    /** Teleport player to an absolute tile index; grants GO salary if passed. */
    virtual void movePlayerTo(Player& player, int tileIndex) = 0;

    /** Reposition player without triggering landedOn or GO salary (used by LassoCard). */
    virtual void repositionPlayer(Player& player, int tileIndex) = 0;

    /** Move player to the nearest tile whose type matches tileType (e.g. "RAILROAD"). */
    virtual void movePlayerToNearest(Player& player, const std::string& tileType) = 0;

    /** Send player directly to jail without passing GO. */
    virtual void sendPlayerToJail(Player& player) = 0;

    // ── Financial operations ───────────────────────────────────────────────────
    /** Credit the GO salary from the bank to the player. */
    virtual void grantSalary(Player& player) = 0;

    /** Transfer money from payer to collector (triggers bankruptcy if insufficient). Shield may block. */
    virtual void transferMoney(Player& payer, Player& collector, int amount) = 0;

    /** Charge amount from player to the bank. Shield may block. Triggers bankruptcy if insufficient. */
    virtual void chargeToBank(Player& player, int amount) = 0;

    /** Charge amount voluntarily (purchase, jail fine). Bypasses shield. Triggers bankruptcy if insufficient. */
    virtual void chargeVoluntary(Player& player, int amount) = 0;

    /** Collect amountPerPlayer from every other active player and give to collector. */
    virtual void collectFromAll(Player& collector, int amountPerPlayer) = 0;

    /** Deduct amountPerPlayer from payer and distribute to every other active player. */
    virtual void payToAll(Player& payer, int amountPerPlayer) = 0;

    // ── Property operations ────────────────────────────────────────────────────
    /** Transfer property ownership to player (free — used for Railroad/Utility). */
    virtual void grantProperty(Player& player, PropertyTile& tile) = 0;

    /** Start auction flow for a property tile. */
    virtual void initiateAuction(PropertyTile& tile) = 0;

    /** Return a property to the bank immediately (used by destructive card effects). */
    virtual void destroyPropertyToBank(PropertyTile& tile) = 0;

    /**
     * @brief Trigger the bankruptcy flow.
     * @param debtor The player who cannot pay.
     * @param creditor The player owed money, or nullptr if the creditor is the bank.
     * @param amount The amount that triggered the bankruptcy.
     */
    virtual void triggerBankruptcy(Player& debtor, Player* creditor, int amount) = 0;
    virtual void refreshMonopolyStatus() = 0;

    // ── UI-mediated interactions (GameEngine delegates to GameView) ────────────
    /** Ask the active player whether to buy the property. Returns true if yes. */
    virtual bool promptBuyProperty(Player& player, PropertyTile& tile) = 0;

    /**
     * @brief Show all opponents' properties and let player pick one to target.
     * @return Pointer to selected PropertyTile, or nullptr if cancelled.
     */
    virtual PropertyTile* promptSelectOpponentProperty(Player& player) = 0;

    /**
     * @brief Let player choose a target opponent.
     * @return Pointer to selected Player, or nullptr if cancelled.
     */
    virtual Player* promptSelectTarget(Player& player) = 0;

    /**
     * @brief Show PPH tax options and return the player's choice (1 = flat, 2 = percentage).
     */
    virtual int promptTaxChoice(Player& player, int flatAmount, int percentage) = 0;

    /**
     * @brief Show all board tiles and let the player pick a destination index.
     * Used by TeleportCard.
     */
    virtual int promptTileIndex(Player& player) = 0;

    /**
     * @brief Let player choose one of their properties to apply festival to.
     * GameEngine resolves the choice and calls StreetTile::activateFestival.
     */
    virtual void promptFestivalSelection(Player& player) = 0;

    /**
     * @brief Prompt a player during an auction for a BID or PASS.
     * @return {true, bidAmount} if the player bids, {false, 0} if they pass.
     */
    virtual std::pair<bool, int> promptAuctionBid(Player& player, int currentBid,
                                                   const PropertyTile& tile) = 0;

    /**
     * @brief Run the interactive liquidation panel when a player must sell assets to cover debt.
     * After this returns, the payment is re-attempted if the player has enough money.
     */
    virtual bool runLiquidationPanel(Player& debtor, int amountNeeded, Player* creditor) = 0;
};
