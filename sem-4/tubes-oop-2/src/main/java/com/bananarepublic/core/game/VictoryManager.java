package com.bananarepublic.core.game;

import com.bananarepublic.config.Constants;
import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.GameWonEvent;
import com.bananarepublic.exception.rule.VictoryConditionException;
import com.bananarepublic.service.ScoreService;

import java.util.Objects;
import java.util.Optional;

public final class VictoryManager {
    private final ScoreService scoreService;
    private final GameEventBus eventBus;

    public VictoryManager(ScoreService scoreService, GameEventBus eventBus) {
        this.scoreService = Objects.requireNonNull(scoreService, "scoreService");
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
    }

    public Optional<AbstractPlayer> checkCurrentPlayerVictory(GameState state, Board board) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");

        if (state.getStatus() == GameStatus.GAME_OVER) {
            return Optional.empty();
        }
        if (state.getStatus() != GameStatus.IN_PROGRESS) {
            throw new VictoryConditionException("Victory can only be checked while the game is in progress");
        }

        AbstractPlayer currentPlayer = state.getCurrentPlayer();
        int points = scoreService.calculateTotalPoints(state, board, currentPlayer);
        if (points < Constants.VICTORY_POINTS_TO_WIN) {
            return Optional.empty();
        }

        state.setStatus(GameStatus.GAME_OVER);
        eventBus.publish(new GameWonEvent(currentPlayer.getId(), points));
        return Optional.of(currentPlayer);
    }
}
