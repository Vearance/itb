#include "GoToJailTile.hpp"
#include "IGameContext.hpp"

using namespace std;

GoToJailTile::GoToJailTile(int id, const string& code, const string& name)
    : SpecialTile(id, code, name) {}

GoToJailTile::~GoToJailTile() {}

void GoToJailTile::executeAction(IGameContext& ctx) {
    ctx.sendPlayerToJail(ctx.getActivePlayer());
}
