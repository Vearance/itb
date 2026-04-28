#include "ActionTile.hpp"
#include "IGameContext.hpp"

using namespace std;

ActionTile::ActionTile(int id, const string& code, const string& name) : Tile(id, code, name) {}

ActionTile::~ActionTile() {}

void ActionTile::landedOn(IGameContext& ctx) {
    executeAction(ctx);
}
