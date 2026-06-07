package com.bananarepublic.config;

public final class GameRules {
    private GameRules() {
    }

    public static boolean isValidPlayerCount(int playerCount) {
        return playerCount >= Constants.MIN_PLAYERS && playerCount <= Constants.MAX_PLAYERS;
    }

    public static void requireValidPlayerCount(int playerCount) {
        if (!isValidPlayerCount(playerCount)) {
            throw new IllegalArgumentException(
                    "Banana Republic requires " + Constants.MIN_PLAYERS + "-" + Constants.MAX_PLAYERS + " players"
            );
        }
    }
}
