#pragma once

#include "AuctionManager.hpp"
#include "BankruptcyHandler.hpp"
#include "Board.hpp"
#include "BoardFactory.hpp"
#include "CommandHandler.hpp"
#include "Deck.hpp"
#include "Dice.hpp"
#include "FinanceManager.hpp"
#include "GameConfig.hpp"
#include "IGameContext.hpp"
#include "Logger.hpp"
#include "PropertyManager.hpp"
#include "TurnManager.hpp"
#include "IUserInteraction.hpp"
#include <memory>
#include <string>
#include <vector>

class Player;
class ChanceCard;
class CommunityChestCard;
class SkillCard;
class GameView;
class ConfigManager;

enum class GameState {
    MENU,
    PLAYING,
    AUCTION,
    LIQUIDATION,
    GAMEOVER
};

/**
 * @brief Top-level game engine — thin orchestrator, implements IGameContext.
 *
 * Responsibilities:
 * - Own the canonical game state (players, board, decks, config).
 * - Implement IGameContext so Tiles and Cards can trigger game events
 *   without coupling to this concrete class.
 * - Delegate turn logic to TurnManager, auctions to AuctionManager,
 *   bankruptcy to BankruptcyHandler, finances to FinanceManager,
 *   property commands to PropertyManager, and command parsing to CommandHandler.
 * - Delegate all I/O to GameView.
 */
class Game : public IGameContext {
    friend class GameSaveLoader;

private:
    // ── State ──────────────────────────────────────────────────────────────────
    std::vector<std::unique_ptr<Player>> players;
    std::unique_ptr<Board> board;
    std::unique_ptr<Dice> dice;

    std::unique_ptr<CardDeck<ChanceCard>> chanceDeck;
    std::unique_ptr<CardDeck<CommunityChestCard>> communityDeck;
    std::unique_ptr<CardDeck<SkillCard>> skillDeck;

    // Card object storage — decks hold raw pointers; these own the card objects
    std::vector<std::unique_ptr<ChanceCard>> chanceCards;
    std::vector<std::unique_ptr<CommunityChestCard>> communityCards;
    std::vector<std::unique_ptr<SkillCard>> allSkillCards;

    GameConfig config;
    std::string currentConfigDir;
    GameState state;
    Logger logger;
    int lastDiceTotal;

    // ── Subsystems ─────────────────────────────────────────────────────────────
    TurnManager turnManager;
    AuctionManager auctionManager;
    BankruptcyHandler bankruptcyHandler;
    FinanceManager financeManager;
    PropertyManager propertyManager;
    CommandHandler commandHandler;

    // ── UI layer (not owned — must outlive Game) ───────────────────────────────
    IUserInteraction* ui; ///< Set via setUserInteraction(); game cannot run without a UI

    // ── Board & turn helpers ───────────────────────────────────────────────────
    std::vector<Player*> getActivePlayers() const;
    void distributeSkillCards();
    void updateMonopolyStatus();
    void resetGameData();

    // ── Per-turn helpers ───────────────────────────────────────────────────────
    void runTurn(Player& player);
    bool handleJailTurn(
        Player& player); ///< Returns true if player exited jail via double (extra turn granted)
    void handleCardDrop(Player& player, SkillCard* newCard);

public:
    Game();
    ~Game() override;

    void setUserInteraction(IUserInteraction* userInteraction);

    // ── Game lifecycle ─────────────────────────────────────────────────────────
    void createGame(const std::string& configDir = "config");
    void loadGame(const std::string& filename);
    void saveGame(const std::string& filename) const;
    void runCycle();
    void handleCommand(const std::string& input, Player& player);

    bool checkWin() const;

    // ── Public accessors used by GameSaveLoader ────────────────────────────────
    const std::vector<std::unique_ptr<Player>>& getPlayers() const;
    const TurnManager& getTurnManager() const;
    const std::vector<std::unique_ptr<SkillCard>>& getAllSkillCards() const;
    GameState getState() const;

    // ── IGameContext implementation ────────────────────────────────────────────
    Player& getActivePlayer() override;
    const std::vector<Player*> getAllActivePlayers() const override;
    Board& getBoard() override;
    const GameConfig& getConfig() const override;
    int getCurrentTurn() const override;
    Logger& getLogger() override;
    int getLastDiceTotal() const override;
    void printMessage(const std::string& message) override;

    void movePlayerBy(Player& player, int steps) override;
    void movePlayerTo(Player& player, int tileIndex) override;
    void repositionPlayer(Player& player, int tileIndex) override;
    void movePlayerToNearest(Player& player, const std::string& tileType) override;
    void sendPlayerToJail(Player& player) override;

    void grantSalary(Player& player) override;
    void transferMoney(Player& payer, Player& collector, int amount) override;
    void chargeToBank(Player& player, int amount) override;
    void chargeVoluntary(Player& player, int amount) override;
    void collectFromAll(Player& collector, int amountPerPlayer) override;
    void payToAll(Player& payer, int amountPerPlayer) override;

    void grantProperty(Player& player, PropertyTile& tile) override;
    void initiateAuction(PropertyTile& tile) override;
    void destroyPropertyToBank(PropertyTile& tile) override;
    void triggerBankruptcy(Player& debtor, Player* creditor, int amount) override;
    void refreshMonopolyStatus() override;

    bool promptBuyProperty(Player& player, PropertyTile& tile) override;
    PropertyTile* promptSelectOpponentProperty(Player& player) override;
    Player* promptSelectTarget(Player& player) override;
    int promptTaxChoice(Player& player, int flat, int pct) override;
    int promptTileIndex(Player& player) override;
    void promptFestivalSelection(Player& player) override;
    std::pair<bool, int> promptAuctionBid(Player& player, int currentBid,
                                          const PropertyTile& tile) override;
    bool runLiquidationPanel(Player& debtor, int amountNeeded, Player* creditor) override;
};
