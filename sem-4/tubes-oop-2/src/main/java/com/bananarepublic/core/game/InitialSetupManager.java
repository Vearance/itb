package com.bananarepublic.core.game;

import com.bananarepublic.core.dice.DiceRoll;
import com.bananarepublic.core.dice.DiceRoller;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.event.DiceRolledEvent;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.PhaseChangedEvent;
import com.bananarepublic.exception.rule.InvalidPhaseException;
import com.bananarepublic.exception.rule.InvalidTurnException;
import com.bananarepublic.validator.TurnValidator;

import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

public final class InitialSetupManager {
    private final GameEventBus eventBus;
    private final TurnManager turnManager;

    public InitialSetupManager(GameEventBus eventBus) {
        this(eventBus, new TurnManager(eventBus));
    }

    public InitialSetupManager(GameEventBus eventBus, TurnManager turnManager) {
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
        this.turnManager = Objects.requireNonNull(turnManager, "turnManager");
    }

    public PlayerId rollStartingPlayer(GameState state, DiceRoller diceRoller) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(diceRoller, "diceRoller");
        TurnValidator.requirePhase(state, GamePhase.DETERMINE_START_PLAYER);

        while (true) {
            Map<PlayerId, DiceRoll> rolls = new LinkedHashMap<>();
            for (AbstractPlayer player : state.getPlayers()) {
                DiceRoll roll = diceRoller.roll();
                rolls.put(player.getId(), roll);
                eventBus.publish(new DiceRolledEvent(player.getId(), roll));
            }

            if (!hasHighestTie(rolls)) {
                return determineStartingPlayer(state, rolls);
            }
        }
    }

    public PlayerId determineStartingPlayer(GameState state, Map<PlayerId, DiceRoll> rolls) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(rolls, "rolls");
        TurnValidator.requirePhase(state, GamePhase.DETERMINE_START_PLAYER);
        requireRollsForEveryPlayer(state, rolls);

        if (hasHighestTie(rolls)) {
            throw new InvalidTurnException("Starting player roll has a tie for the highest total");
        }

        PlayerId startingPlayerId = rolls.entrySet().stream()
                .max(Comparator.comparingInt(entry -> entry.getValue().getTotal()))
                .orElseThrow(() -> new InvalidTurnException("No dice rolls were provided"))
                .getKey();
        state.setCurrentPlayerIndex(indexOfPlayer(state, startingPlayerId));
        state.resetInitialPlacementProgress();
        changePhase(state, GamePhase.INITIAL_PLACEMENT_FORWARD);
        return startingPlayerId;
    }

    public void advanceInitialPlacement(GameState state) {
        Objects.requireNonNull(state, "state");
        if (state.getPhase() == GamePhase.INITIAL_PLACEMENT_FORWARD) {
            advanceForwardPlacement(state);
            return;
        }
        if (state.getPhase() == GamePhase.INITIAL_PLACEMENT_REVERSE) {
            advanceReversePlacement(state);
            return;
        }
        throw new InvalidPhaseException("Initial placement can only advance during setup placement phases");
    }

    private void advanceForwardPlacement(GameState state) {
        state.incrementInitialForwardPlacementCount();
        if (state.getInitialForwardPlacementCount() < state.getPlayers().size()) {
            state.setCurrentPlayerIndex(nextPlayerIndex(state));
            return;
        }
        changePhase(state, GamePhase.INITIAL_PLACEMENT_REVERSE);
    }

    private void advanceReversePlacement(GameState state) {
        state.incrementInitialReversePlacementCount();
        if (state.getInitialReversePlacementCount() < state.getPlayers().size()) {
            state.setCurrentPlayerIndex(previousPlayerIndex(state));
            return;
        }
        turnManager.startNormalTurns(state);
    }

    private void requireRollsForEveryPlayer(GameState state, Map<PlayerId, DiceRoll> rolls) {
        if (rolls.size() != state.getPlayers().size()) {
            throw new InvalidTurnException("Every player must roll before determining the starting player");
        }
        for (AbstractPlayer player : state.getPlayers()) {
            DiceRoll roll = rolls.get(player.getId());
            if (roll == null) {
                throw new InvalidTurnException("Missing starting roll for player " + player.getId().getValue());
            }
        }
    }

    private boolean hasHighestTie(Map<PlayerId, DiceRoll> rolls) {
        int highest = rolls.values().stream()
                .mapToInt(DiceRoll::getTotal)
                .max()
                .orElseThrow(() -> new InvalidTurnException("No dice rolls were provided"));
        return rolls.values().stream()
                .filter(roll -> roll.getTotal() == highest)
                .count() > 1;
    }

    private int nextPlayerIndex(GameState state) {
        return (state.getCurrentPlayerIndex() + 1) % state.getPlayers().size();
    }

    private int previousPlayerIndex(GameState state) {
        int playerCount = state.getPlayers().size();
        return (state.getCurrentPlayerIndex() - 1 + playerCount) % playerCount;
    }

    private int indexOfPlayer(GameState state, PlayerId playerId) {
        List<AbstractPlayer> players = state.getPlayers();
        for (int i = 0; i < players.size(); i++) {
            if (players.get(i).getId().equals(playerId)) {
                return i;
            }
        }
        throw new InvalidTurnException("Player is not part of this game: " + playerId.getValue());
    }

    private void changePhase(GameState state, GamePhase nextPhase) {
        GamePhase previousPhase = state.getPhase();
        state.setPhase(nextPhase);
        eventBus.publish(new PhaseChangedEvent(previousPhase, nextPhase));
    }
}
