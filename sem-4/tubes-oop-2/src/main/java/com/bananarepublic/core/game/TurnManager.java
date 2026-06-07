package com.bananarepublic.core.game;

import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.PhaseChangedEvent;
import com.bananarepublic.event.PlayerTurnEndedEvent;
import com.bananarepublic.event.PlayerTurnStartedEvent;
import com.bananarepublic.validator.TurnValidator;

import java.util.Objects;

public final class TurnManager {
    private final GameEventBus eventBus;

    public TurnManager(GameEventBus eventBus) {
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
    }

    // called once after the initial placement phase
    public void startNormalTurns(GameState state) {
        Objects.requireNonNull(state, "state");
        TurnValidator.requireStatus(state, GameStatus.NOT_STARTED);
        TurnValidator.requireInitialPlacementComplete(state);
        state.setTurnNumber(1);
        state.resetExperimentCardPlayedThisTurn();
        state.setStatus(GameStatus.IN_PROGRESS);
        changePhase(state, GamePhase.ROLL_DICE);
        publishTurnStarted(state);
    }

    // called at the end of each player's turn
    public void endTurn(GameState state) {
        Objects.requireNonNull(state, "state");
        TurnValidator.requireCanEndTurn(state);

        eventBus.publish(new PlayerTurnEndedEvent(state.getCurrentPlayer().getId(), state.getTurnNumber()));

        int previousPlayerIndex = state.getCurrentPlayerIndex();
        int previousTurnNumber = state.getTurnNumber();
        state.setCurrentPlayerIndex(nextPlayerIndex(state));

        state.setTurnNumber(state.getTurnNumber() + 1);  //increment turn number
        state.resetExperimentCardPlayedThisTurn();
        changePhase(state, GamePhase.ROLL_DICE);

        assert state.getCurrentPlayerIndex() == (previousPlayerIndex + 1) % state.getPlayers().size()
                : "End turn must advance to the next player";
        assert state.getTurnNumber() == previousTurnNumber + 1 : "End turn must increment turn number by one";
        publishTurnStarted(state);
    }

    public boolean canEndTurn(GameState state) {
        return TurnValidator.canEndTurn(state);
    }

    // clockwise turn order
    public int nextPlayerIndex(GameState state) {
        Objects.requireNonNull(state, "state");
        return (state.getCurrentPlayerIndex() + 1) % state.getPlayers().size();
    }

    private void publishTurnStarted(GameState state) {
        eventBus.publish(new PlayerTurnStartedEvent(state.getCurrentPlayer().getId(), state.getTurnNumber()));
    }

    private void changePhase(GameState state, GamePhase nextPhase) {
        GamePhase previousPhase = state.getPhase();
        state.setPhase(nextPhase);
        eventBus.publish(new PhaseChangedEvent(previousPhase, nextPhase));
    }

}
