#pragma once

#include <string>
#include <vector>

class Player;
class IGameContext;
class Logger;
class GameConfig;
class IUserInteraction;

/**
 * @brief Handles all financial operations: salary, transfers, charges, and
 * multi-player payments.
 *
 * Single Responsibility: money movement between players and the bank.
 * Extracted from Game to keep the orchestrator thin.
 */
class FinanceManager {
public:
    FinanceManager() = default;
    ~FinanceManager() = default;

    /** Credit the GO salary from the bank to the player. */
    void grantSalary(Player& player, const GameConfig& config, int currentTurn, Logger& logger,
                     IUserInteraction* ui);

    /** Transfer money from payer to collector (triggers bankruptcy if insufficient). Shield may block. */
    void transferMoney(Player& payer, Player& collector, int amount, int currentTurn,
                       Logger& logger, IGameContext& ctx, IUserInteraction* ui);

    /** Charge amount from player to the bank. Shield may block. Triggers bankruptcy if insufficient. */
    void chargeToBank(Player& player, int amount, int currentTurn, Logger& logger,
                      IGameContext& ctx, IUserInteraction* ui);

    /** Charge amount voluntarily (purchase, jail fine). Bypasses shield. Triggers bankruptcy if insufficient. */
    void chargeVoluntary(Player& player, int amount, int currentTurn, Logger& logger,
                         IGameContext& ctx, IUserInteraction* ui);

    /** Collect amountPerPlayer from every other active player and give to collector. */
    void collectFromAll(Player& collector, int amountPerPlayer,
                        const std::vector<Player*>& activePlayers, int currentTurn, Logger& logger,
                        IGameContext& ctx, IUserInteraction* ui);

    /** Deduct amountPerPlayer from payer and distribute to every other active player. */
    void payToAll(Player& payer, int amountPerPlayer, const std::vector<Player*>& activePlayers,
                  int currentTurn, Logger& logger, IGameContext& ctx, IUserInteraction* ui);
};
