package com.bananarepublic.core.bot.strategy;

import com.bananarepublic.config.CardCostConfig;
import com.bananarepublic.core.bot.Action;
import com.bananarepublic.core.bot.PlayerStrategy;
import com.bananarepublic.core.game.GamePhase;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;

public final class GreedyBotStrategy implements PlayerStrategy {
    @Override
    public String getStrategyName() {
        return "Greedy Bot";
    }

    @Override
    public String getDescription() {
        return "Buys an experiment card when possible, otherwise trades surplus resources with the bank.";
    }

    @Override
    public Action takeTurn(GameState state) {
        if (state.getPhase() != GamePhase.PLAYER_ACTIONS) {
            return Action.noOp();
        }

        AbstractPlayer player = state.getCurrentPlayer();
        if (player.getInventory().hasResources(CardCostConfig.getExperimentCardCost())
                && state.getDevelopmentDeck().remainingCount() > 0) {
            return Action.buyExperimentCard();
        }

        for (ResourceType offeredType : ResourceType.values()) {
            if (player.getResourceCount(offeredType) < 4) {
                continue;
            }

            ResourceType requestedType = chooseNeededResource(player, offeredType);
            if (requestedType != null && state.getBank().hasResource(requestedType, 1)) {
                return Action.harborTrade(offeredType, requestedType);
            }
        }

        return Action.endTurn();
    }

    private ResourceType chooseNeededResource(AbstractPlayer player, ResourceType offeredType) {
        ResourceBundle cardCost = CardCostConfig.getExperimentCardCost();
        for (ResourceType type : ResourceType.values()) {
            if (type != offeredType && player.getResourceCount(type) < cardCost.get(type)) {
                return type;
            }
        }
        for (ResourceType type : ResourceType.values()) {
            if (type != offeredType) {
                return type;
            }
        }
        return null;
    }
}
