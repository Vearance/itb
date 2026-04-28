#include "TeleportCard.hpp"
#include "Board.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"

using namespace std;

TeleportCard::TeleportCard(const string& name, const string& description)
    : SkillCard(name, description) {}

TeleportCard::~TeleportCard() {}

void TeleportCard::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    int destIdx = ctx.promptTileIndex(player);
    ctx.repositionPlayer(player, destIdx);
    ctx.printMessage("TeleportCard: pindah ke " + ctx.getBoard().getTileAt(destIdx)->getName() +
                     ".\n");
    ctx.getBoard().getTileAt(destIdx)->landedOn(ctx);
}
