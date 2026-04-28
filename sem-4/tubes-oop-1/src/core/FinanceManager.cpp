#include "FinanceManager.hpp"
#include "GameConfig.hpp"
#include "IGameContext.hpp"
#include "IUserInteraction.hpp"
#include "Logger.hpp"
#include "Player.hpp"

#include <string>

using namespace std;

void FinanceManager::grantSalary(Player& player, const GameConfig& config, int currentTurn,
                                 Logger& logger, IUserInteraction* ui) {
    player += config.goSalary;
    if (ui)
        ui->printMessage(player.getUsername() + " melewati Petak Mulai! Terima gaji M" +
                         to_string(config.goSalary) + ". Uang: M" + to_string(player.getMoney()) +
                         "\n");
    logger.logEvent(LogLevel::INFO, currentTurn, player.getUsername(), "GAJI",
                    "Terima gaji GO M" + to_string(config.goSalary));
}

void FinanceManager::transferMoney(Player& payer, Player& collector, int amount, int currentTurn,
                                   Logger& logger, IGameContext& ctx, IUserInteraction* ui) {
    if (payer.isShielded()) {
        if (ui)
            ui->printMessage("[SHIELD] " + payer.getUsername() +
                             " dilindungi dari tagihan M" + to_string(amount) + "!\n");
        return;
    }
    if (payer.getMoney() < amount) {
        ctx.triggerBankruptcy(payer, &collector, amount);
        return;
    }
    payer -= amount;
    collector += amount;
    if (ui)
        ui->printMessage(payer.getUsername() + " membayar M" + to_string(amount) + " ke " +
                         collector.getUsername() + ". Uang " + payer.getUsername() + ": M" +
                         to_string(payer.getMoney()) + "\n");
    logger.logEvent(LogLevel::INFO, currentTurn, payer.getUsername(), "TRANSFER",
                    "Bayar M" + to_string(amount) + " ke " + collector.getUsername());
}

void FinanceManager::chargeToBank(Player& player, int amount, int currentTurn, Logger& logger,
                                  IGameContext& ctx, IUserInteraction* ui) {
    if (player.isShielded()) {
        if (ui)
            ui->printMessage("[SHIELD] " + player.getUsername() +
                             " dilindungi dari biaya M" + to_string(amount) + "!\n");
        return;
    }
    if (player.getMoney() < amount) {
        ctx.triggerBankruptcy(player, nullptr, amount);
        return;
    }
    player -= amount;
    if (ui)
        ui->printMessage(player.getUsername() + " membayar M" + to_string(amount) +
                         " ke Bank. Uang: M" + to_string(player.getMoney()) + "\n");
    logger.logEvent(LogLevel::INFO, currentTurn, player.getUsername(), "BANK",
                    "Bayar ke Bank M" + to_string(amount));
}

void FinanceManager::chargeVoluntary(Player& player, int amount, int currentTurn, Logger& logger,
                                     IGameContext& ctx, IUserInteraction* ui) {
    if (player.getMoney() < amount) {
        ctx.triggerBankruptcy(player, nullptr, amount);
        return;
    }
    player -= amount;
    if (ui)
        ui->printMessage(player.getUsername() + " membayar M" + to_string(amount) +
                         " ke Bank. Uang: M" + to_string(player.getMoney()) + "\n");
    logger.logEvent(LogLevel::INFO, currentTurn, player.getUsername(), "BANK",
                    "Bayar sukarela M" + to_string(amount));
}

void FinanceManager::collectFromAll(Player& collector, int amountPerPlayer,
                                    const vector<Player*>& activePlayers, int currentTurn,
                                    Logger& logger, IGameContext& ctx, IUserInteraction* ui) {
    for (Player* p : activePlayers) {
        if (p == &collector)
            continue;
        transferMoney(*p, collector, amountPerPlayer, currentTurn, logger, ctx, ui);
    }
}

void FinanceManager::payToAll(Player& payer, int amountPerPlayer,
                              const vector<Player*>& activePlayers, int currentTurn,
                              Logger& logger, IGameContext& ctx, IUserInteraction* ui) {
    for (Player* p : activePlayers) {
        if (p == &payer)
            continue;
        transferMoney(payer, *p, amountPerPlayer, currentTurn, logger, ctx, ui);
    }
}
