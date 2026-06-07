package com.bananarepublic.service;

import com.bananarepublic.config.CardCostConfig;
import com.bananarepublic.core.action.MoveRobberAction;
import com.bananarepublic.core.action.PlayCardAction;
import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.board.Intersection;
import com.bananarepublic.core.board.IntersectionId;
import com.bananarepublic.core.board.Path;
import com.bananarepublic.core.board.PathId;
import com.bananarepublic.core.card.CardType;
import com.bananarepublic.core.card.ExperimentCard;
import com.bananarepublic.core.card.ExperimentCardId;
import com.bananarepublic.core.card.OwnedExperimentCard;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.CardBoughtEvent;
import com.bananarepublic.event.CardPlayedEvent;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.ResourceChangedEvent;
import com.bananarepublic.exception.card.CardNotOwnedException;
import com.bananarepublic.exception.card.InvalidCardPlayException;
import com.bananarepublic.validator.CardPlayValidator;

import java.util.List;
import java.util.Objects;

public final class CardService {
    private static final int ROAD_BUILDING_PIPE_COUNT = 2;

    private final GameEventBus eventBus;
    private final CardPlayValidator cardPlayValidator;
    private final BuildService buildService;
    private final LargestArmyService largestArmyService;
    private final VictoryPointService victoryPointService;
    private final RobberService robberService;

    public CardService(GameEventBus eventBus) {
        this(eventBus, new CardPlayValidator(), new BuildService(eventBus), new LargestArmyService(), new VictoryPointService(), new RobberService(eventBus));
    }

    public CardService(
            GameEventBus eventBus,
            CardPlayValidator cardPlayValidator,
            BuildService buildService,
            LargestArmyService largestArmyService,
            VictoryPointService victoryPointService,
            RobberService robberService
    ) {
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
        this.cardPlayValidator = Objects.requireNonNull(cardPlayValidator, "cardPlayValidator");
        this.buildService = Objects.requireNonNull(buildService, "buildService");
        this.largestArmyService = Objects.requireNonNull(largestArmyService, "largestArmyService");
        this.victoryPointService = Objects.requireNonNull(victoryPointService, "victoryPointService");
        this.robberService = Objects.requireNonNull(robberService, "robberService");
    }

    public OwnedExperimentCard buyExperimentCard(GameState state) {
        Objects.requireNonNull(state, "state");
        cardPlayValidator.validateCanBuyCard(state);

        AbstractPlayer player = state.getCurrentPlayer();
        ResourceBundle cost = CardCostConfig.getExperimentCardCost();
        player.getInventory().spendResourceBundle(cost);
        state.getBank().receiveResourceBundle(cost);
        publishResourceChanges(player, cost, -1);

        ExperimentCard card = state.getDevelopmentDeck().draw();
        OwnedExperimentCard ownedCard = player.getCardHand().addCard(card, state.getTurnNumber());
        if (card.getType() == CardType.VICTORY_POINT) {
            victoryPointService.addHiddenVictoryPoint(state, player.getId());
        }

        eventBus.publish(new CardBoughtEvent(player.getId(), card.getId(), card.getType(), state.getDevelopmentDeck().remainingCount()));
        return ownedCard;
    }

    public void playCard(GameState state, Board board, PlayCardAction action) {
        Objects.requireNonNull(action, "action");
        if (!state.getCurrentPlayer().getId().equals(action.getPlayerId())) {
            throw new InvalidCardPlayException("Only the active player can play an experiment card");
        }

        OwnedExperimentCard ownedCard = requireOwnedCard(state.getCurrentPlayer(), action.getCardId());
        switch (ownedCard.getType()) {
            case KNIGHT -> playKnightCard(state, board, action.getCardId(), action.getRobberHexTileId(), action.getTargetPlayerId());
            case ROAD_BUILDING -> playRoadBuildingCard(state, board, action.getCardId(), action.getFreePipePathIds());
            case MONOPOLY -> playMonopolyCard(state, action.getCardId(), action.getMonopolyResourceType());
            case VICTORY_POINT -> throw new InvalidCardPlayException("Victory Point cards are not played manually");
            case PLUGIN -> playPluginCard(state, action.getCardId());
        }
    }

    public void validateCanPlayCard(GameState state, ExperimentCardId cardId, CardType expectedType) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(cardId, "cardId");
        Objects.requireNonNull(expectedType, "expectedType");
        requirePlayableCard(state, state.getCurrentPlayer(), cardId, expectedType);
    }

    public void playKnightCard(GameState state, Board board, ExperimentCardId cardId, String newRobberHexTileId, PlayerId targetPlayerId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(cardId, "cardId");
        if (newRobberHexTileId == null || newRobberHexTileId.isBlank()) {
            throw new InvalidCardPlayException("Knight card must choose a Nimon Ungu destination");
        }

        AbstractPlayer player = state.getCurrentPlayer();
        OwnedExperimentCard ownedCard = requirePlayableCard(state, player, cardId, CardType.KNIGHT);
        robberService.moveRobberFromCard(
                state,
                board,
                new MoveRobberAction(player.getId(), newRobberHexTileId, targetPlayerId)
        );
        largestArmyService.recordKnightPlayed(state, player.getId());
        finishPlayedCard(state, player, ownedCard);
    }

    public void playRoadBuildingCard(GameState state, Board board, ExperimentCardId cardId, List<PathId> pathIds) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(cardId, "cardId");
        Objects.requireNonNull(pathIds, "pathIds");
        if (pathIds.size() != ROAD_BUILDING_PIPE_COUNT) {
            throw new InvalidCardPlayException("Road Building must place exactly two free pipes");
        }

        AbstractPlayer player = state.getCurrentPlayer();
        OwnedExperimentCard ownedCard = requirePlayableCard(state, player, cardId, CardType.ROAD_BUILDING);
        validateRoadBuildingPair(state, board, player, pathIds);
        buildService.buildFreePipe(state, board, pathIds.get(0));
        buildService.buildFreePipe(state, board, pathIds.get(1));
        finishPlayedCard(state, player, ownedCard);
    }

    public void validateCanPlayRoadBuildingCard(GameState state, Board board, ExperimentCardId cardId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(cardId, "cardId");

        AbstractPlayer player = state.getCurrentPlayer();
        requirePlayableCard(state, player, cardId, CardType.ROAD_BUILDING);
        validateRoadBuildingSupply(player);
        if (!hasTwoStepRoadBuildingPlacement(state, board, player)) {
            throw new InvalidCardPlayException("Tidak ada dua lokasi Pipa valid untuk Road Building");
        }
    }

    public void validateRoadBuildingFirstPipe(GameState state, Board board, ExperimentCardId cardId, PathId pathId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(cardId, "cardId");
        Objects.requireNonNull(pathId, "pathId");

        AbstractPlayer player = state.getCurrentPlayer();
        requirePlayableCard(state, player, cardId, CardType.ROAD_BUILDING);
        validateRoadBuildingSupply(player);
        buildService.validateCanBuildFreePipe(state, board, pathId);

        Path firstPath = requirePath(board, pathId);
        boolean hasSecondPipe = board.getPaths().values().stream()
                .anyMatch(secondPath -> canBuildFreePipeAfterVirtualPipe(board, secondPath, firstPath, player.getId()));
        if (!hasSecondPipe) {
            throw new InvalidCardPlayException("Pilih Pipa lain: setelah Pipa ini tidak ada Pipa kedua yang valid");
        }
    }

    public void placeRoadBuildingPipe(GameState state, Board board, ExperimentCardId cardId, PathId pathId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(cardId, "cardId");
        Objects.requireNonNull(pathId, "pathId");

        requirePlayableCard(state, state.getCurrentPlayer(), cardId, CardType.ROAD_BUILDING);
        buildService.buildFreePipe(state, board, pathId);
    }

    public void completeRoadBuildingCard(GameState state, ExperimentCardId cardId, int placedPipeCount) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(cardId, "cardId");
        if (placedPipeCount != ROAD_BUILDING_PIPE_COUNT) {
            throw new InvalidCardPlayException("Road Building must place exactly two free pipes");
        }

        AbstractPlayer player = state.getCurrentPlayer();
        OwnedExperimentCard ownedCard = requirePlayableCard(state, player, cardId, CardType.ROAD_BUILDING);
        finishPlayedCard(state, player, ownedCard);
    }

    public void playMonopolyCard(GameState state, ExperimentCardId cardId, ResourceType resourceType) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(cardId, "cardId");
        Objects.requireNonNull(resourceType, "resourceType");

        AbstractPlayer player = state.getCurrentPlayer();
        OwnedExperimentCard ownedCard = requirePlayableCard(state, player, cardId, CardType.MONOPOLY);
        int collectedAmount = 0;

        for (AbstractPlayer otherPlayer : state.getPlayers()) {
            if (otherPlayer.getId().equals(player.getId())) {
                continue;
            }

            int amount = otherPlayer.getResourceCount(resourceType);
            if (amount == 0) {
                continue;
            }

            otherPlayer.removeResource(resourceType, amount);
            collectedAmount += amount;
            eventBus.publish(new ResourceChangedEvent(otherPlayer.getId(), resourceType, -amount, otherPlayer.getResourceCount(resourceType)));
        }

        if (collectedAmount > 0) {
            player.addResource(resourceType, collectedAmount);
            eventBus.publish(new ResourceChangedEvent(player.getId(), resourceType, collectedAmount, player.getResourceCount(resourceType)));
        }
        finishPlayedCard(state, player, ownedCard);
    }

    public void playPluginCard(GameState state, ExperimentCardId cardId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(cardId, "cardId");

        AbstractPlayer player = state.getCurrentPlayer();
        OwnedExperimentCard ownedCard = requirePlayableCard(state, player, cardId, CardType.PLUGIN);
        ownedCard.getCard().applyEffect(state, player);
        finishPlayedCard(state, player, ownedCard);
    }

    private OwnedExperimentCard requirePlayableCard(GameState state, AbstractPlayer player, ExperimentCardId cardId, CardType expectedType) {
        OwnedExperimentCard ownedCard = requireOwnedCard(player, cardId);
        cardPlayValidator.validateCanPlayCard(state, player, ownedCard, expectedType);
        return ownedCard;
    }

    private void validateRoadBuildingPair(GameState state, Board board, AbstractPlayer player, List<PathId> pathIds) {
        validateRoadBuildingSupply(player);
        if (pathIds.get(0).equals(pathIds.get(1))) {
            throw new InvalidCardPlayException("Path Pipa gratis harus berbeda");
        }

        buildService.validateCanBuildFreePipe(state, board, pathIds.get(0));
        Path firstPath = requirePath(board, pathIds.get(0));
        Path secondPath = requirePath(board, pathIds.get(1));
        if (!canBuildFreePipeAfterVirtualPipe(board, secondPath, firstPath, player.getId())) {
            throw new InvalidCardPlayException("Pipa gratis kedua tidak valid setelah Pipa pertama");
        }
    }

    private void validateRoadBuildingSupply(AbstractPlayer player) {
        if (player.getSupply().getPipes() < ROAD_BUILDING_PIPE_COUNT) {
            throw new InvalidCardPlayException("Road Building membutuhkan minimal 2 supply Pipa");
        }
    }

    private boolean hasTwoStepRoadBuildingPlacement(GameState state, Board board, AbstractPlayer player) {
        for (Path firstPath : board.getPaths().values()) {
            if (!canBuildFreePipe(state, board, firstPath.getId())) {
                continue;
            }
            for (Path secondPath : board.getPaths().values()) {
                if (canBuildFreePipeAfterVirtualPipe(board, secondPath, firstPath, player.getId())) {
                    return true;
                }
            }
        }
        return false;
    }

    private boolean canBuildFreePipe(GameState state, Board board, PathId pathId) {
        try {
            buildService.validateCanBuildFreePipe(state, board, pathId);
            return true;
        } catch (RuntimeException ignored) {
            return false;
        }
    }

    private boolean canBuildFreePipeAfterVirtualPipe(Board board, Path secondPath, Path firstPath, PlayerId playerId) {
        if (secondPath.hasPipe() || secondPath.getId().equals(firstPath.getId())) {
            return false;
        }
        return isConnectedAtEndpointAfterVirtualPipe(board, secondPath.getFirstIntersectionId(), firstPath, playerId)
                || isConnectedAtEndpointAfterVirtualPipe(board, secondPath.getSecondIntersectionId(), firstPath, playerId);
    }

    private boolean isConnectedAtEndpointAfterVirtualPipe(Board board, IntersectionId endpointId, Path virtualPipe, PlayerId playerId) {
        Intersection endpoint = board.getIntersections().get(endpointId);
        if (endpoint == null) {
            return false;
        }

        if (endpoint.getBuilding()
                .map(building -> building.getOwnerId().equals(playerId))
                .orElse(false)) {
            return true;
        }

        boolean blockedByOpponentBuilding = endpoint.getBuilding()
                .map(building -> !building.getOwnerId().equals(playerId))
                .orElse(false);
        return !blockedByOpponentBuilding
                && (hasOwnedPipeTouching(board, endpointId, playerId) || virtualPipe.touches(endpointId));
    }

    private boolean hasOwnedPipeTouching(Board board, IntersectionId intersectionId, PlayerId playerId) {
        for (Path path : board.getPaths().values()) {
            if (!path.touches(intersectionId)) {
                continue;
            }
            if (path.getPipe()
                    .map(pipe -> pipe.getOwnerId().equals(playerId))
                    .orElse(false)) {
                return true;
            }
        }
        return false;
    }

    private Path requirePath(Board board, PathId pathId) {
        Path path = board.getPaths().get(pathId);
        if (path == null) {
            throw new InvalidCardPlayException("Path tidak ditemukan: " + pathId.value());
        }
        return path;
    }

    private OwnedExperimentCard requireOwnedCard(AbstractPlayer player, ExperimentCardId cardId) {
        return player.getCardHand()
                .findCard(cardId)
                .orElseThrow(() -> new CardNotOwnedException("Player does not own this experiment card"));
    }

    private void finishPlayedCard(GameState state, AbstractPlayer player, OwnedExperimentCard ownedCard) {
        player.getCardHand().removeCard(ownedCard.getCardId());
        state.markExperimentCardPlayedThisTurn();
        eventBus.publish(new CardPlayedEvent(player.getId(), ownedCard.getCardId(), ownedCard.getType()));
    }

    private void publishResourceChanges(AbstractPlayer player, ResourceBundle resources, int direction) {
        for (ResourceType type : ResourceType.values()) {
            int amount = resources.get(type);
            if (amount == 0) {
                continue;
            }
            eventBus.publish(new ResourceChangedEvent(
                    player.getId(),
                    type,
                    amount * direction,
                    player.getResourceCount(type)
            ));
        }
    }
}
