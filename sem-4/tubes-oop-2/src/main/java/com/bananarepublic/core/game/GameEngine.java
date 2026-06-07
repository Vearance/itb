package com.bananarepublic.core.game;

import com.bananarepublic.core.action.DiscardResourceAction;
import com.bananarepublic.core.action.MoveRobberAction;
import com.bananarepublic.core.action.PlayCardAction;
import com.bananarepublic.core.action.TradeAction;
import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.board.IntersectionId;
import com.bananarepublic.core.board.PathId;
import com.bananarepublic.core.card.ExperimentCardId;
import com.bananarepublic.core.card.OwnedExperimentCard;
import com.bananarepublic.core.dice.DiceRoll;
import com.bananarepublic.core.dice.DiceRoller;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.PhaseChangedEvent;
import com.bananarepublic.service.BuildService;
import com.bananarepublic.service.CardService;
import com.bananarepublic.service.DiscardService;
import com.bananarepublic.service.DiceService;
import com.bananarepublic.service.InitialSetupService;
import com.bananarepublic.service.ResourceProductionService;
import com.bananarepublic.service.RobberService;
import com.bananarepublic.service.TradeService;

import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

public final class GameEngine {
    private final GameState state;
    private final Board board;
    private final InitialSetupService initialSetupService;
    private final BuildService buildService;
    private final TurnManager turnManager;
    private final VictoryManager victoryManager;
    private final DiceService diceService;
    private final ResourceProductionService resourceProductionService;
    private final TradeService tradeService;
    private final CardService cardService;
    private final DiscardService discardService;
    private final RobberService robberService;
    private final GameEventBus eventBus;

    public GameEngine(
            GameState state,
            Board board,
            InitialSetupService initialSetupService,
            BuildService buildService,
            TurnManager turnManager,
            VictoryManager victoryManager,
            DiceService diceService,
            ResourceProductionService resourceProductionService,
            TradeService tradeService,
            CardService cardService,
            DiscardService discardService,
            RobberService robberService,
            GameEventBus eventBus
    ) {
        this.state = Objects.requireNonNull(state, "state");
        this.board = Objects.requireNonNull(board, "board");
        this.initialSetupService = Objects.requireNonNull(initialSetupService, "initialSetupService");
        this.buildService = Objects.requireNonNull(buildService, "buildService");
        this.turnManager = Objects.requireNonNull(turnManager, "turnManager");
        this.victoryManager = Objects.requireNonNull(victoryManager, "victoryManager");
        this.diceService = Objects.requireNonNull(diceService, "diceService");
        this.resourceProductionService = Objects.requireNonNull(resourceProductionService, "resourceProductionService");
        this.tradeService = Objects.requireNonNull(tradeService, "tradeService");
        this.cardService = Objects.requireNonNull(cardService, "cardService");
        this.discardService = Objects.requireNonNull(discardService, "discardService");
        this.robberService = Objects.requireNonNull(robberService, "robberService");
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
    }

    public GameState getState() {
        return state;
    }

    public Board getBoard() {
        return board;
    }

    public PlayerId rollStartingPlayer(DiceRoller diceRoller) {
        return initialSetupService.rollStartingPlayer(state, diceRoller);
    }

    public PlayerId determineStartingPlayer(Map<PlayerId, DiceRoll> rolls) {
        return initialSetupService.determineStartingPlayer(state, rolls);
    }

    public void placeInitialSettlementAndPipe(IntersectionId intersectionId, PathId pathId) {
        initialSetupService.placeInitialSettlementAndPipe(state, board, intersectionId, pathId);
    }

    public void startNormalTurns() {
        turnManager.startNormalTurns(state);
    }

    public Optional<AbstractPlayer> buildPipe(PathId pathId) {
        buildService.buildPipe(state, board, pathId);
        tradeService.cancelPendingNormalTrade("Build action completed");
        return checkCurrentPlayerVictory();
    }

    public Optional<AbstractPlayer> buildSettlement(IntersectionId intersectionId) {
        buildService.buildSettlement(state, board, intersectionId);
        tradeService.cancelPendingNormalTrade("Build action completed");
        return checkCurrentPlayerVictory();
    }

    public Optional<AbstractPlayer> upgradeSettlementToLaboratory(IntersectionId intersectionId) {
        buildService.upgradeSettlementToLaboratory(state, board, intersectionId);
        tradeService.cancelPendingNormalTrade("Build action completed");
        return checkCurrentPlayerVictory();
    }

    public Optional<AbstractPlayer> checkCurrentPlayerVictory() {
        return victoryManager.checkCurrentPlayerVictory(state, board);
    }

    public DiceRoll rollDice() {
        DiceRoll diceRoll = diceService.rollDice(state);
        resolveDiceRoll(diceRoll);
        return diceRoll;
    }

    public DiceRoll rollDice(DiceRoller diceRoller) {
        Objects.requireNonNull(diceRoller, "diceRoller");
        DiceRoll diceRoll = new DiceService(diceRoller, eventBus).rollDice(state);
        resolveDiceRoll(diceRoll);
        return diceRoll;
    }

    private void resolveDiceRoll(DiceRoll diceRoll) {
        if (diceRoll.isSeven()) {
            tradeService.cancelPendingNormalTrade("Rolled 7");
            state.resetPlayersWhoDiscarded();
            changePhase(discardService.hasPendingDiscards(state) ? GamePhase.DISCARD_RESOURCES : GamePhase.MOVE_ROBBER);
        } else {
            resourceProductionService.produceResources(state, board, diceRoll.getTotal());
            changePhase(GamePhase.PLAYER_ACTIONS);
        }
    }

    public void endTurn() {
        if (turnManager.canEndTurn(state)) {
            tradeService.cancelPendingNormalTrade("Turn ended");
        }
        turnManager.endTurn(state);
    }

    public TradeAction requestNormalTrade(PlayerId targetId, ResourceBundle offeredResources, ResourceBundle requestedResources) {
        TradeAction action = new TradeAction(state.getCurrentPlayer().getId(), targetId, offeredResources, requestedResources);
        return tradeService.requestNormalTrade(state, action);
    }

    public void acceptNormalTrade(TradeAction action) {
        tradeService.acceptNormalTrade(state, action);
    }

    public void acceptNormalTrade(PlayerId acceptingPlayerId, TradeAction action) {
        tradeService.acceptNormalTrade(state, action, acceptingPlayerId);
    }

    public void rejectNormalTrade(TradeAction action, String reason) {
        tradeService.rejectNormalTrade(action, reason);
    }

    public void rejectNormalTrade(PlayerId rejectingPlayerId, TradeAction action, String reason) {
        tradeService.rejectNormalTrade(action, rejectingPlayerId, reason);
    }

    public void tradeWithHarbor(ResourceType offeredType, ResourceType requestedType) {
        tradeService.tradeWithHarbor(state, board, offeredType, requestedType);
        tradeService.cancelPendingNormalTrade("Harbor trade completed");
    }

    public List<AbstractPlayer> getPlayersRequiredToDiscard() {
        return discardService.getPlayersRequiredToDiscard(state);
    }

    public int getRequiredDiscardCount(AbstractPlayer player) {
        return discardService.getRequiredDiscardCount(player);
    }

    public void discardResources(PlayerId playerId, ResourceBundle discardedResources) {
        discardResources(new DiscardResourceAction(playerId, discardedResources));
    }

    public void discardResources(DiscardResourceAction action) {
        discardService.discardResources(state, action);
        state.markPlayerDiscarded(action.getPlayerId());
        if (!discardService.hasPendingDiscards(state)) {
            changePhase(GamePhase.MOVE_ROBBER);
        }
    }

    public List<AbstractPlayer> getValidRobberStealTargets(String hexTileId) {
        return robberService.getValidStealTargets(state, board, state.getCurrentPlayer().getId(), hexTileId);
    }

    public Optional<ResourceType> moveRobber(String targetHexTileId, PlayerId targetPlayerId) {
        return moveRobber(new MoveRobberAction(state.getCurrentPlayer().getId(), targetHexTileId, targetPlayerId));
    }

    public Optional<ResourceType> moveRobber(MoveRobberAction action) {
        Optional<ResourceType> stolenResource = robberService.moveRobberAfterSeven(state, board, action);
        changePhase(GamePhase.PLAYER_ACTIONS);
        return stolenResource;
    }

    public OwnedExperimentCard buyExperimentCard() {
        OwnedExperimentCard ownedCard = cardService.buyExperimentCard(state);
        tradeService.cancelPendingNormalTrade("Card bought");
        checkCurrentPlayerVictory();
        return ownedCard;
    }

    public Optional<AbstractPlayer> playCard(PlayCardAction action) {
        cardService.playCard(state, board, action);
        tradeService.cancelPendingNormalTrade("Card played");
        return checkCurrentPlayerVictory();
    }

    public Optional<AbstractPlayer> playKnightCard(ExperimentCardId cardId, String newRobberHexTileId, PlayerId targetPlayerId) {
        cardService.playKnightCard(state, board, cardId, newRobberHexTileId, targetPlayerId);
        tradeService.cancelPendingNormalTrade("Card played");
        return checkCurrentPlayerVictory();
    }

    public Optional<AbstractPlayer> playRoadBuildingCard(ExperimentCardId cardId, List<PathId> pathIds) {
        cardService.playRoadBuildingCard(state, board, cardId, pathIds);
        tradeService.cancelPendingNormalTrade("Card played");
        return checkCurrentPlayerVictory();
    }

    public void validateCanPlayRoadBuildingCard(ExperimentCardId cardId) {
        cardService.validateCanPlayRoadBuildingCard(state, board, cardId);
    }

    public void validateRoadBuildingFirstPipe(ExperimentCardId cardId, PathId pathId) {
        cardService.validateRoadBuildingFirstPipe(state, board, cardId, pathId);
    }

    public void placeRoadBuildingPipe(ExperimentCardId cardId, PathId pathId) {
        cardService.placeRoadBuildingPipe(state, board, cardId, pathId);
        tradeService.cancelPendingNormalTrade("Card played");
    }

    public Optional<AbstractPlayer> completeRoadBuildingCard(ExperimentCardId cardId, int placedPipeCount) {
        cardService.completeRoadBuildingCard(state, cardId, placedPipeCount);
        tradeService.cancelPendingNormalTrade("Card played");
        return checkCurrentPlayerVictory();
    }

    public Optional<AbstractPlayer> playMonopolyCard(ExperimentCardId cardId, ResourceType resourceType) {
        cardService.playMonopolyCard(state, cardId, resourceType);
        tradeService.cancelPendingNormalTrade("Card played");
        return checkCurrentPlayerVictory();
    }

    private void changePhase(GamePhase nextPhase) {
        GamePhase previousPhase = state.getPhase();
        state.setPhase(nextPhase);
        eventBus.publish(new PhaseChangedEvent(previousPhase, nextPhase));
    }
}
