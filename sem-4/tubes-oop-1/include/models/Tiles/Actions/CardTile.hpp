#pragma once
#include "ActionTile.hpp"

class ICardDeck;

/**
 * @brief Tile that causes the landing player to draw from a card deck.
 * The deck is injected at construction (dependency injection), so
 * CardTile does not depend on the concrete Game or deck type.
 */
class CardTile : public ActionTile {
private:
    ICardDeck& deck; ///< Chance or Community Chest deck — injected, not owned

public:
    CardTile(int id, const std::string& code, const std::string& name, ICardDeck& deck);
    ~CardTile() override;

protected:
    void executeAction(IGameContext& ctx) override;
};
