package com.bananarepublic.core.game;

import com.bananarepublic.config.Constants;
import com.bananarepublic.config.GameRules;
import com.bananarepublic.core.card.DevelopmentDeck;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBank;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;

public final class GameState {
    private final List<AbstractPlayer> players;
    private final ResourceBank bank;
    private final DevelopmentDeck developmentDeck;
    private GamePhase phase;
    private GameStatus status;
    private int currentPlayerIndex;
    private int turnNumber;
    private int initialForwardPlacementCount;
    private int initialReversePlacementCount;
    private boolean experimentCardPlayedThisTurn;
    private PlayerId longestRoadOwnerId;  // min 5
    private PlayerId largestArmyOwnerId;  // min 3
    private int turnTimerRemainingSeconds;
    private boolean turnTimerRunning;
    private final Set<PlayerId> playersWhoDiscarded;


    public GameState(List<AbstractPlayer> players, ResourceBank bank) {
        this(players, bank, DevelopmentDeck.standardDeck());
    }

    public GameState(List<AbstractPlayer> players, ResourceBank bank, DevelopmentDeck developmentDeck) {
        GameRules.requireValidPlayerCount(players.size());
        this.players = new ArrayList<>(players);
        this.bank = Objects.requireNonNull(bank, "bank");
        this.developmentDeck = Objects.requireNonNull(developmentDeck, "developmentDeck");
        this.phase = GamePhase.DETERMINE_START_PLAYER;
        this.status = GameStatus.NOT_STARTED;
        this.currentPlayerIndex = 0;
        this.turnNumber = 0;
        this.initialForwardPlacementCount = 0;
        this.initialReversePlacementCount = 0;
        this.experimentCardPlayedThisTurn = false;
        this.turnTimerRemainingSeconds = Constants.TURN_TIMER_SECONDS;
        this.turnTimerRunning = false;
        this.playersWhoDiscarded = new HashSet<>();
    }

    public static GameState newGame(List<AbstractPlayer> players) {
        return new GameState(players, new ResourceBank());
    }

    public List<AbstractPlayer> getPlayers() {
        return Collections.unmodifiableList(players);
    }

    public List<AbstractPlayer> getAllPlayers() {
        return getPlayers();
    }

    public AbstractPlayer getCurrentPlayer() {
        return players.get(currentPlayerIndex);
    }

    public Optional<AbstractPlayer> findPlayerById(PlayerId playerId) {
        Objects.requireNonNull(playerId, "playerId");
        return players.stream()
                .filter(player -> player.getId().equals(playerId))
                .findFirst();
    }

    public AbstractPlayer getPlayerById(PlayerId playerId) {
        return findPlayerById(playerId).orElseThrow(() -> new IllegalArgumentException("Player is not part of this game: " + playerId));
    }

    public int getCurrentPlayerIndex() {
        return currentPlayerIndex;
    }

    public void setCurrentPlayerIndex(int currentPlayerIndex) {
        if (currentPlayerIndex < 0 || currentPlayerIndex >= players.size()) {
            throw new IllegalArgumentException("Current player index is out of range");
        }
        this.currentPlayerIndex = currentPlayerIndex;
    }

    public int getTurnNumber() {
        return turnNumber;
    }

    public int getRoundNumber() {
        if (turnNumber == 0) {
            return 0;
        }
        return ((turnNumber - 1) / players.size()) + 1;
    }

    public void setTurnNumber(int turnNumber) {
        if (turnNumber < 0) {
            throw new IllegalArgumentException("Turn number cannot be negative");
        }
        this.turnNumber = turnNumber;
    }

    public int getInitialForwardPlacementCount() {
        return initialForwardPlacementCount;
    }

    public void incrementInitialForwardPlacementCount() {
        initialForwardPlacementCount++;
    }

    public int getInitialReversePlacementCount() {
        return initialReversePlacementCount;
    }

    public void incrementInitialReversePlacementCount() {
        initialReversePlacementCount++;
    }

    public void setInitialForwardPlacementCount(int count) {
        if (count < 0) {
            throw new IllegalArgumentException("Initial forward placement count cannot be negative");
        }
        this.initialForwardPlacementCount = count;
    }

    public void setInitialReversePlacementCount(int count) {
        if (count < 0) {
            throw new IllegalArgumentException("Initial reverse placement count cannot be negative");
        }
        this.initialReversePlacementCount = count;
    }

    public void resetInitialPlacementProgress() {
        initialForwardPlacementCount = 0;
        initialReversePlacementCount = 0;
    }

    public ResourceBank getBank() {
        return bank;
    }

    public DevelopmentDeck getDevelopmentDeck() {
        return developmentDeck;
    }

    public boolean hasPlayedExperimentCardThisTurn() {
        return experimentCardPlayedThisTurn;
    }

    public void markExperimentCardPlayedThisTurn() {
        experimentCardPlayedThisTurn = true;
    }

    public void resetExperimentCardPlayedThisTurn() {
        experimentCardPlayedThisTurn = false;
    }

    public PlayerId getLongestRoadOwnerId() {
        return longestRoadOwnerId;
    }

    public void setLongestRoadOwnerId(PlayerId longestRoadOwnerId) {
        this.longestRoadOwnerId = Objects.requireNonNull(longestRoadOwnerId, "longestRoadOwnerId");
    }

    public void clearLongestRoadOwner() {
        this.longestRoadOwnerId = null;
    }

    public PlayerId getLargestArmyOwnerId() {
        return largestArmyOwnerId;
    }

    public void setLargestArmyOwnerId(PlayerId largestArmyOwnerId) {
        this.largestArmyOwnerId = Objects.requireNonNull(largestArmyOwnerId, "largestArmyOwnerId");
    }

    public void clearLargestArmyOwner() {
        this.largestArmyOwnerId = null;
    }

    public GamePhase getPhase() {
        return phase;
    }

    public void setPhase(GamePhase phase) {
        this.phase = Objects.requireNonNull(phase, "phase");
        assert this.phase != null : "Game phase cannot be null after assignment";
    }

    public GameStatus getStatus() {
        return status;
    }

    public void setStatus(GameStatus status) {
        this.status = Objects.requireNonNull(status, "status");
    }

    public int getTurnTimerRemainingSeconds() {
        return turnTimerRemainingSeconds;
    }

    public void setTurnTimerRemainingSeconds(int turnTimerRemainingSeconds) {
        if (turnTimerRemainingSeconds < 0) {
            throw new IllegalArgumentException("Turn timer remaining seconds cannot be negative");
        }
        this.turnTimerRemainingSeconds = turnTimerRemainingSeconds;
    }

    public boolean isTurnTimerRunning() {
        return turnTimerRunning;
    }

    public void setTurnTimerRunning(boolean turnTimerRunning) {
        this.turnTimerRunning = turnTimerRunning;
    }

    public void resetTurnTimerState() {
        turnTimerRemainingSeconds = Constants.TURN_TIMER_SECONDS;
        turnTimerRunning = false;
    }

    public void markPlayerDiscarded(PlayerId playerId) {
        playersWhoDiscarded.add(Objects.requireNonNull(playerId, "playerId"));
    }

    public boolean hasPlayerDiscarded(PlayerId playerId) {
        return playersWhoDiscarded.contains(playerId);
    }

    public Set<PlayerId> getPlayersWhoDiscarded() {
        return Collections.unmodifiableSet(playersWhoDiscarded);
    }

    public void resetPlayersWhoDiscarded() {
        playersWhoDiscarded.clear();
    }
}
