package com.bananarepublic.validator;

import com.bananarepublic.core.game.GamePhase;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.game.GameStatus;
import com.bananarepublic.core.player.PlayerId;

import java.util.Objects;

public final class TurnValidator {
    private TurnValidator() {
    }

    public static void requireStatus(GameState state, GameStatus expectedStatus) {
        Objects.requireNonNull(state, "state");
        if (state.getStatus() != expectedStatus) {
            throw new IllegalStateException("Expected game status " + expectedStatus);
        }
    }

    public static void requirePhase(GameState state, GamePhase expectedPhase) {
        Objects.requireNonNull(state, "state");
        if (state.getPhase() != expectedPhase) {
            throw new IllegalStateException("Expected game phase " + expectedPhase);
        }
    }

    public static void requireInitialPlacementComplete(GameState state) {
        Objects.requireNonNull(state, "state");
        if (state.getPhase() != GamePhase.INITIAL_PLACEMENT_REVERSE
                || state.getInitialForwardPlacementCount() < state.getPlayers().size()
                || state.getInitialReversePlacementCount() < state.getPlayers().size()) {
            throw new IllegalStateException("Normal turns can only start after initial placement is complete");
        }
    }

    public static void requireCurrentPlayer(GameState state, PlayerId playerId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(playerId, "playerId");
        if (!state.getCurrentPlayer().getId().equals(playerId)) {
            throw new IllegalStateException("It is not this player's turn: " + playerId);
        }
    }

    public static boolean canEndTurn(GameState state) {
        Objects.requireNonNull(state, "state");
        return state.getStatus() == GameStatus.IN_PROGRESS
                && state.getPhase() == GamePhase.PLAYER_ACTIONS;
    }

    public static void requireCanEndTurn(GameState state) {
        if (!canEndTurn(state)) {
            throw new IllegalStateException("Player can only end turn during player actions");
        }
    }
}
