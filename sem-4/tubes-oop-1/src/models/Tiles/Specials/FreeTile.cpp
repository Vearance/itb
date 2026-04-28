#include "FreeTile.hpp"
#include "IGameContext.hpp"

using namespace std;

FreeTile::FreeTile(int id, const string& code, const string& name)
    : SpecialTile(id, code, name) {}

FreeTile::~FreeTile() {}

void FreeTile::executeAction(IGameContext& /*ctx*/) {
}
