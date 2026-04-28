#include "SpecialTile.hpp"
#include "IGameContext.hpp"

using namespace std;

SpecialTile::SpecialTile(int id, const string& code, const string& name) : Tile(id, code, name) {}

SpecialTile::~SpecialTile() {}

void SpecialTile::landedOn(IGameContext& ctx) {
    executeAction(ctx);
}
