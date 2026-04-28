#pragma once

class Player;
class IGameContext;

/**
 * @brief Handles all bankruptcy scenarios: liquidation panel, asset transfer, and auction trigger.
 * Single Responsibility: bankruptcy logic only.
 */
class BankruptcyHandler {
public:
    BankruptcyHandler();
    ~BankruptcyHandler();

    /**
     * @brief Entry point called when a player cannot meet a payment obligation.
     *
     * 1. Calculates maximum achievable funds via full liquidation.
     * 2. If achievable → open liquidation panel; player must liquidate until covered.
     * 3. If not achievable → declare bankrupt; transfer assets to creditor (or bank).
     *
     * @param debtor The player who cannot pay.
     * @param creditor The player owed money, or nullptr if the bank is the creditor.
     * @param amount The amount that triggered the bankruptcy check.
     * @param ctx Game context for asset transfers, auctions, and state mutation.
     */
    void handle(Player& debtor, Player* creditor, int amount, IGameContext& ctx);

private:
    /// @brief Calculate the maximum total cash obtainable by liquidating all of debtor's assets.
    int computeMaxLiquidation(const Player& debtor) const;

    /// @brief Transfer all debtor's assets to creditor (player bankruptcy).
    void transferAssetsToPlayer(Player& debtor, Player& creditor, IGameContext& ctx);

    /// @brief Return all debtor's assets to the bank and trigger per-property auctions.
    void returnAssetsToBank(Player& debtor, IGameContext& ctx);
};
