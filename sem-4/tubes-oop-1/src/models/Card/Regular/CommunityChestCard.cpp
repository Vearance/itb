#include "CommunityChestCard.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"
#include <string>

using namespace std;

CommunityChestCard::CommunityChestCard(const string& name, const string& description,
                                       CommunityChestEffect effect)
    : Card(name, description), effect(effect) {}

CommunityChestCard::~CommunityChestCard() {}

CommunityChestEffect CommunityChestCard::getEffect() const {
    return effect;
}

void CommunityChestCard::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    ctx.printMessage("Kartu Dana Umum: " + getName() + "\n");
    ctx.printMessage("Efek: " + getDescription() + "\n");

    switch (effect) {
    case CommunityChestEffect::BIRTHDAY_COLLECT_100:
        // Collect 100 from every other active player
        ctx.collectFromAll(player, 100);
        ctx.printMessage(player.getUsername() + " menerima M100 dari setiap pemain.\n");
        break;

    case CommunityChestEffect::DOCTOR_FEE_700:
        // Pay 700 to the bank; chargeToBank triggers bankruptcy if insufficient
        ctx.chargeToBank(player, 700);
        ctx.printMessage(player.getUsername() + " membayar M700 biaya dokter ke Bank.\n");
        break;

    case CommunityChestEffect::CAMPAIGN_PAY_200:
        // Pay 200 to every other active player
        ctx.payToAll(player, 200);
        ctx.printMessage(player.getUsername() + " membayar M200 ke setiap pemain.\n");
        break;
    }
}
