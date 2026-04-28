#include "FestivalTile.hpp"
#include "IGameContext.hpp"

using namespace std;

FestivalTile::FestivalTile(int id, const string& code, const string& name)
    : ActionTile(id, code, name) {}

FestivalTile::~FestivalTile() {}

void FestivalTile::executeAction(IGameContext& ctx) {
    ctx.promptFestivalSelection(ctx.getActivePlayer());
}
