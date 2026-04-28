#include "JailTile.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"

using namespace std;

JailTile::JailTile(int id, const string& code, const string& name, int fineAmount)
    : SpecialTile(id, code, name), fineAmount(fineAmount) {}

JailTile::~JailTile() {}

int JailTile::getFineAmount() const {
    return fineAmount;
}

void JailTile::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    if (!player.isInJail()) {
        return;
    }
}
