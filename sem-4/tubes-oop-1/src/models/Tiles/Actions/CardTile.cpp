#include "CardTile.hpp"
#include "Deck.hpp"
#include "IGameContext.hpp"
#include "Card.hpp"
#include "Logger.hpp"
#include "Player.hpp"

using namespace std;

CardTile::CardTile(int id, const string& code, const string& name, ICardDeck& deck)
    : ActionTile(id, code, name), deck(deck) {}

CardTile::~CardTile() {}

void CardTile::executeAction(IGameContext& ctx) {
    Card* card = deck.draw();
    if (!card)
        return;
    ctx.getLogger().logEvent(LogLevel::INFO, ctx.getCurrentTurn(),
                             ctx.getActivePlayer().getUsername(), "CARD_DRAW",
                             getCode() + " -> " + card->getName());
    card->executeAction(ctx);
    deck.discard(card);
}
