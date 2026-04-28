#include "LassoCard.hpp"
#include "Board.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"

using namespace std;

LassoCard::LassoCard(const string& name, const string& description)
    : SkillCard(name, description) {}

LassoCard::~LassoCard() {}

void LassoCard::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    // Find the nearest opponent ahead on the board and pull them to player's position
    Player* target = ctx.promptSelectTarget(player);
    if (target) {
        ctx.repositionPlayer(*target, player.getPosition());
        ctx.printMessage("LassoCard: " + target->getUsername() + " ditarik ke petak " +
                         ctx.getBoard().getTileAt(player.getPosition())->getName() + "!\n");
    }
}
