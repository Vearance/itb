
#include "BankruptcyHandler.hpp"
#include "IGameContext.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "PropertyTile.hpp"
#include "StreetTile.hpp"

using namespace std;

BankruptcyHandler::BankruptcyHandler() {}

BankruptcyHandler::~BankruptcyHandler() {}

void BankruptcyHandler::handle(Player& debtor, Player* creditor, int amount, IGameContext& ctx) {
    int maxCash = computeMaxLiquidation(debtor);

    if (debtor.getMoney() + maxCash >= amount) {
        ctx.getLogger().logEvent(LogLevel::INFO, ctx.getCurrentTurn(), debtor.getUsername(),
                                 "LIKUIDASI",
                                 "Wajib likuidasi untuk kewajiban M" + to_string(amount));
        bool paid = ctx.runLiquidationPanel(debtor, amount, creditor);
        if (paid)
            return;
    }
    debtor.setStatus(PlayerStatus::BANKRUPT);
    ctx.getLogger().logEvent(
        LogLevel::INFO, ctx.getCurrentTurn(), debtor.getUsername(), "BANKRUPT",
        string("Bangkrut kepada ") + (creditor ? creditor->getUsername() : "BANK") +
            " untuk kewajiban M" + to_string(amount));
    if (creditor) {
        transferAssetsToPlayer(debtor, *creditor, ctx);
    } else {
        returnAssetsToBank(debtor, ctx);
    }
}

int BankruptcyHandler::computeMaxLiquidation(const Player& debtor) const {
    int total = 0;
    for (const PropertyTile* prop : debtor.getProperties()) {
        if (prop->isMortgaged()) {
            continue;
        }

        total += prop->getPrice();
        const auto* street = dynamic_cast<const StreetTile*>(prop);
        if (street) {
            int level = street->getPropertyLevel();
            if (level == 5) {
                total += (street->getHotelPrice() + 4 * street->getHousePrice()) / 2;
            } else {
                total += level * street->getHousePrice() / 2;
            }
        }
    }
    return total;
}

void BankruptcyHandler::transferAssetsToPlayer(Player& debtor, Player& creditor,
                                               IGameContext& ctx) {
    const auto debtorProperties = debtor.getProperties();
    const int propertyCount = static_cast<int>(debtorProperties.size());
    const int remainingCash = debtor.getMoney();
    for (PropertyTile* prop : debtorProperties) {
        const bool wasMortgaged = prop->isMortgaged();
        ctx.grantProperty(creditor, *prop);
        if (wasMortgaged) {
            prop->setStatus(PropertyStatus::MORTGAGED);
        }
    }
    creditor += debtor.getMoney();
    debtor -= debtor.getMoney();
    ctx.refreshMonopolyStatus();
    ctx.getLogger().logEvent(LogLevel::INFO, ctx.getCurrentTurn(), creditor.getUsername(),
                             "AMBIL_ASET",
                             "Ambil " + to_string(propertyCount) + " properti dan uang M" +
                                 to_string(remainingCash) + " dari " + debtor.getUsername());
}

void BankruptcyHandler::returnAssetsToBank(Player& debtor, IGameContext& ctx) {
    const auto debtorProperties = debtor.getProperties();
    const int propertyCount = static_cast<int>(debtorProperties.size());
    const int remainingCash = debtor.getMoney();
    for (PropertyTile* prop : debtorProperties) {
        if (auto* street = dynamic_cast<StreetTile*>(prop)) {
            street->setPropertyLevel(0);
        }
        prop->setFestivalState(1, 0);
        debtor.removeProperty(prop);
        prop->releaseToBank();
    }
    ctx.refreshMonopolyStatus();

    for (PropertyTile* prop : debtorProperties) {
        ctx.initiateAuction(*prop);
    }

    debtor -= debtor.getMoney();
    ctx.getLogger().logEvent(LogLevel::INFO, ctx.getCurrentTurn(), debtor.getUsername(),
                             "ASET_BANK",
                             "Serahkan uang M" + to_string(remainingCash) + " dan " +
                                 to_string(propertyCount) +
                                 " properti ke Bank untuk dilelang");
}
