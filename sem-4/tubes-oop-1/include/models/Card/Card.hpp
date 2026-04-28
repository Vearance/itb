#pragma once
#include <string>

// Forward declaration — Card does not depend on the concrete Game class
class IGameContext;

/// @brief Abstract base class for all cards in the game.
class Card {
private:
    int id;
    std::string name;
    std::string description;

public:
    Card(const std::string& name, const std::string& description);
    virtual ~Card();

    int getId() const;
    const std::string& getName() const;
    const std::string& getDescription() const;

    /**
     * @brief Execute this card's effect within the given game context.
     * @param ctx Live game context — use to move players, transfer money, etc.
     */
    virtual void executeAction(IGameContext& ctx) = 0;
};
