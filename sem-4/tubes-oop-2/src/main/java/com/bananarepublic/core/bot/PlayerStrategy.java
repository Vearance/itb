package com.bananarepublic.core.bot;

import com.bananarepublic.core.game.GameState;

public interface PlayerStrategy {
    default String getStrategyId() {
        return "BOT-" + getClass().getName().replaceAll("[^A-Za-z0-9._-]", "_");
    }

    default String getStrategyName() {
        return getClass().getSimpleName();
    }

    default String getDescription() {
        return "A Banana Republic bot strategy.";
    }

    Action takeTurn(GameState state);
}
