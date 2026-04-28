#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Forward declarations — keeps this interface lightweight
class Player;
class PropertyTile;
class Board;
class Logger;

/**
 * @brief Pure interface for all user interaction (input/output).
 *
 * This is the *only* boundary between the game engine and the terminal.
 * No code in core/ or models/ should include <iostream> or use std::cin/std::cout.
 */
class IUserInteraction {
public:
    virtual ~IUserInteraction() = default;

    // ── Generic I/O ────────────────────────────────────────────────────────────
    virtual void printMessage(const std::string& msg) = 0;
    virtual void printEmptyLine() = 0;
    virtual std::string readLine() = 0;
    virtual int readInt() = 0;

    // ── Setup ──────────────────────────────────────────────────────────────────
    virtual std::vector<std::pair<std::string, bool>> promptPlayerSetup() = 0;

    // ── Board / state display ──────────────────────────────────────────────────
    virtual void displayBoard(const Board& board, const std::vector<Player*>& players,
                              int currentTurn, int maxTurn) const = 0;
    virtual void printPropertyDeed(const PropertyTile& property) = 0;
    virtual void printPlayerProperties(const Player& player) = 0;
    virtual void printTransactionLogs(const std::vector<std::string>& logs) const = 0;
    virtual void showEndGameScreen(const std::vector<std::string>& winners,
                                   const std::vector<std::string>& finalRankings) const = 0;

    // ── Turn commands ──────────────────────────────────────────────────────────
    virtual std::string getCommandInput(const Player& activePlayer) const = 0;

    // ── Prompts (previously in IGameContext) ───────────────────────────────────
    virtual bool promptBuyProperty(Player& player, PropertyTile& tile) = 0;
    virtual PropertyTile* promptSelectOpponentProperty(Player& player,
                                                       const std::vector<Player*>& players,
                                                       const Board& board) = 0;
    virtual Player* promptSelectTarget(Player& player, const std::vector<Player*>& players,
                                       const Board& board) = 0;
    virtual int promptTaxChoice(Player& player, int flatAmount, int percentage) = 0;
    virtual int promptTileIndex(Player& player, const Board& board) = 0;
    virtual void promptFestivalSelection(Player& player) = 0;
    virtual std::pair<bool, int> promptAuctionBid(Player& player, int currentBid,
                                                  const PropertyTile& tile) = 0;
    virtual bool runLiquidationPanel(Player& debtor, int amountNeeded, Player* creditor,
                                     const std::vector<Player*>& players, const Board& board,
                                     int currentTurn, Logger& logger) = 0;
};
