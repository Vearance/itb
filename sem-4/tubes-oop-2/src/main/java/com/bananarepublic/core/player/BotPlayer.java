package com.bananarepublic.core.player;

import com.bananarepublic.core.bot.PlayerStrategy;

import java.util.Objects;

public final class BotPlayer extends AbstractPlayer {
    private final PlayerStrategy strategy;

    public BotPlayer(PlayerId id, String name, PlayerColor color, PlayerStrategy strategy) {
        super(id, name, color);
        this.strategy = Objects.requireNonNull(strategy, "strategy");
    }

    public PlayerStrategy getStrategy() {
        return strategy;
    }
}
