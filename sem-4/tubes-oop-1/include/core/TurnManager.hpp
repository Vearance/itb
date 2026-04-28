#pragma once
#include <vector>

class Player;

/**
 * @brief Manages turn order, the active player index, and turn counter.
 * Single Responsibility: knows who plays when, nothing else.
 */
class TurnManager {
private:
    std::vector<int> turnOrder; ///< Player indices in play order (randomised at game start)
    int activeIndex;
    int currentTurn;
    int maxTurn;

public:
    TurnManager(int maxTurn);
    ~TurnManager();

    /// @brief Randomise turn order from 0..playerCount-1 at game start.
    void initOrder(int playerCount);

    /// @brief Load a pre-determined order (e.g. from save file).
    void loadOrder(const std::vector<int>& order, int activeIndex, int currentTurn);

    /// @brief Advance to the next active player, increment turn counter when a full round completes.
    void advance(const std::vector<Player*>& activePlayers);

    /// @brief Remove a player from the turn order (on bankruptcy).
    void removePlayer(int playerIndex);

    int getActivePlayerIndex() const;
    int getCurrentTurn() const;
    int getMaxTurn() const;

    const std::vector<int>& getTurnOrder() const;

    /// @brief True when currentTurn > maxTurn (and maxTurn > 0).
    bool hasReachedMaxTurn() const;
};
