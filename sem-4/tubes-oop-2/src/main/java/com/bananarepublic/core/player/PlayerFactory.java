package com.bananarepublic.core.player;

import com.bananarepublic.config.GameRules;
import com.bananarepublic.core.bot.PlayerStrategy;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public final class PlayerFactory {
    private PlayerFactory() {
    }

    public static HumanPlayer createHumanPlayer(int playerNumber, String name, PlayerColor color) {
        if (playerNumber < 1) {
            throw new IllegalArgumentException("Player number must start from 1");
        }
        return new HumanPlayer(new PlayerId(playerId(playerNumber)), name, color);
    }

    public static BotPlayer createBotPlayer(int playerNumber, String name, PlayerColor color, PlayerStrategy strategy) {
        if (playerNumber < 1) {
            throw new IllegalArgumentException("Player number must start from 1");
        }
        return new BotPlayer(new PlayerId(playerId(playerNumber)), name, color, Objects.requireNonNull(strategy, "strategy"));
    }

    public static List<AbstractPlayer> createDefaultHumanPlayers(List<String> names) {
        GameRules.requireValidPlayerCount(names.size());
        PlayerColor[] colors = PlayerColor.values();
        List<AbstractPlayer> players = new ArrayList<>();
        for (int i = 0; i < names.size(); i++) {
            players.add(createHumanPlayer(i + 1, names.get(i), colors[i]));
        }
        return players;
    }

    private static String playerId(int playerNumber) {
        return "P" + playerNumber;
    }
}
