#pragma once

#include "IUserInteraction.hpp"
#include <memory>
#include <string>
#include <vector>

#include "Board.hpp"
#include "Player.hpp"
#include "PropertyTile.hpp"

/// @brief A terminal-based presentation layer that renders the game state and handles user inputs.
class GameView : public IUserInteraction {
private:
    /// @brief Clears the screen dynamically based on the terminal operating system.
    void clearScreen() const;

public:
    /// @brief Creates the View interface component.
    GameView();
    ~GameView() override;

    // ── Generic output ─────────────────────────────────────────────────────────
    void printMessage(const std::string& msg) override;
    void printEmptyLine() override;

    /// @brief Prints the main menu of the game where users can start a new game or load a saved
    /// one.
    void displayMainMenu() const;

    /// @brief Prompts for username settings and number of players at the beginning of the setup.
    /// @return A vector of strings containing player names and bool isComputer.
    std::vector<std::pair<std::string, bool>> promptPlayerSetup() override;

    /// @brief Renders the visual representation of the game board.
    /// @param board Const reference to the board logic.
    /// @param players Const reference to the list of players on the board.
    void displayBoard(const Board& board, const std::vector<Player*>& players, int currentTurn = -1,
                      int maxTurn = 50) const override;

    /// @brief Renders an individual property tile detail and its ownership statuses.
    /// @param property Referencing a property tile component.
    void printPropertyDeed(const PropertyTile& property) override;

    /// @brief Renders the available properties controlled by a precise player.
    /// @param player The protagonist user owning the properties.
    void printPlayerProperties(const Player& player) override;

    /// @brief Displays an ongoing auction panel layout.
    /// @param currentBid The highest ongoing offered price.
    /// @param highBidder The username of the user leading the auction.
    /// @param tile The property tile currently in the auction round.
    void renderAuctionPanel(int currentBid, const std::string& highBidder,
                            const PropertyTile& tile) const;

    /// @brief Renders the interactive command prompt containing context actions for a specific
    /// player's turn.
    /// @param activePlayer Reference to the player whose turn is ongoing.
    /// @return String representation of the submitted and trimmed input.
    std::string getCommandInput(const Player& activePlayer) const override;

    std::string readLine() override;
    int readInt() override;

    /// @brief Prints the history log entries to the terminal console.
    /// @param logs Collection of string histories formatted in lines.
    void printTransactionLogs(const std::vector<std::string>& logs) const override;

    /// @brief Declares visually the ending sequences and rankings.
    /// @param winners A collection of the winner player instances or names.
    /// @param finalRankings Collection storing leaderboard metadata string summaries.
    void showEndGameScreen(const std::vector<std::string>& winners,
                           const std::vector<std::string>& finalRankings) const override;

    // ── Prompts (moved from IGameContext) ──────────────────────────────────────
    bool promptBuyProperty(Player& player, PropertyTile& tile) override;
    PropertyTile* promptSelectOpponentProperty(Player& player, const std::vector<Player*>& players,
                                               const Board& board) override;
    Player* promptSelectTarget(Player& player, const std::vector<Player*>& players,
                               const Board& board) override;
    int promptTaxChoice(Player& player, int flatAmount, int percentage) override;
    int promptTileIndex(Player& player, const Board& board) override;
    void promptFestivalSelection(Player& player) override;
    std::pair<bool, int> promptAuctionBid(Player& player, int currentBid,
                                          const PropertyTile& tile) override;
    bool runLiquidationPanel(Player& debtor, int amountNeeded, Player* creditor,
                             const std::vector<Player*>& players, const Board& board,
                             int currentTurn, Logger& logger) override;
};
