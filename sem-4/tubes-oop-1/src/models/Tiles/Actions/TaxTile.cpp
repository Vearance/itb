#include "TaxTile.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"

using namespace std;

TaxTile::TaxTile(int id, const string& code, const string& name,
                 TaxType taxType, int baseAmount, int percentage)
    : ActionTile(id, code, name), taxType(taxType), baseAmount(baseAmount), percentage(percentage) {
}

TaxTile::~TaxTile() {}

TaxType TaxTile::getTaxType() const {
    return taxType;
}

int TaxTile::getBaseAmount() const {
    return baseAmount;
}

int TaxTile::getPercentage() const {
    return percentage;
}

void TaxTile::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    if (taxType == TaxType::PBM) {
        ctx.chargeToBank(player, baseAmount);
        return;
    }
    int choice = ctx.promptTaxChoice(player, baseAmount, percentage);
    int amount;
    if (choice == 2) {
        amount = player.getTotalWealth() * percentage / 100;
    } else {
        amount = baseAmount;
    }
    ctx.chargeToBank(player, amount);
}
