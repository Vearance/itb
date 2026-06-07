package com.bananarepublic.service;

import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.board.HexTile;
import com.bananarepublic.core.board.Intersection;
import com.bananarepublic.core.board.IntersectionId;
import com.bananarepublic.core.board.Path;
import com.bananarepublic.core.board.PathId;
import com.bananarepublic.core.building.AbstractBuilding;
import com.bananarepublic.core.building.BuildType;
import com.bananarepublic.core.building.BuildingFactory;
import com.bananarepublic.core.building.Pipe;
import com.bananarepublic.core.dice.DiceRoll;
import com.bananarepublic.core.dice.DiceRoller;
import com.bananarepublic.core.game.GamePhase;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.game.InitialSetupManager;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.BuildingPlacedEvent;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.ResourceChangedEvent;
import com.bananarepublic.event.ResourceProducedEvent;
import com.bananarepublic.exception.build.InvalidPlacementException;
import com.bananarepublic.validator.BuildValidator;

import java.util.Map;
import java.util.Objects;

public final class InitialSetupService {
    private final InitialSetupManager setupManager;
    private final BuildValidator buildValidator;
    private final GameEventBus eventBus;

    public InitialSetupService(GameEventBus eventBus) {
        this(new InitialSetupManager(eventBus), new BuildValidator(), eventBus);
    }

    public InitialSetupService(InitialSetupManager setupManager, GameEventBus eventBus) {
        this(setupManager, new BuildValidator(), eventBus);
    }

    public InitialSetupService(InitialSetupManager setupManager, BuildValidator buildValidator, GameEventBus eventBus) {
        this.setupManager = Objects.requireNonNull(setupManager, "setupManager");
        this.buildValidator = Objects.requireNonNull(buildValidator, "buildValidator");
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
    }

    public PlayerId rollStartingPlayer(GameState state, DiceRoller diceRoller) {
        return setupManager.rollStartingPlayer(state, diceRoller);
    }

    public PlayerId determineStartingPlayer(GameState state, Map<PlayerId, DiceRoll> rolls) {
        return setupManager.determineStartingPlayer(state, rolls);
    }

    public void placeInitialSettlementAndPipe(GameState state, Board board, IntersectionId intersectionId, PathId pathId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(intersectionId, "intersectionId");
        Objects.requireNonNull(pathId, "pathId");

        requireInitialPlacementPhase(state);
        AbstractPlayer player = state.getCurrentPlayer();
        requireSupply(player, BuildType.SETTLEMENT);
        requireSupply(player, BuildType.PIPE);
        buildValidator.validateInitialSettlement(board, intersectionId);
        buildValidator.validateInitialPipe(board, pathId, intersectionId);

        Intersection intersection = buildValidator.requireIntersection(board, intersectionId);
        Path path = buildValidator.requirePath(board, pathId);
        AbstractBuilding settlement = BuildingFactory.create(BuildType.SETTLEMENT, player.getId());
        AbstractBuilding pipe = BuildingFactory.create(BuildType.PIPE, player.getId());

        intersection.placeBuilding(settlement);
        path.placePipe((Pipe) pipe);
        player.getSupply().use(BuildType.SETTLEMENT);
        player.getSupply().use(BuildType.PIPE);
        player.getScore().addPublicPoints(settlement.getVictoryPoints());

        eventBus.publish(new BuildingPlacedEvent(settlement, intersectionId.value()));
        eventBus.publish(new BuildingPlacedEvent(pipe, pathId.value()));

        if (state.getPhase() == GamePhase.INITIAL_PLACEMENT_REVERSE) {
            giveStartingResources(state, board, player, intersection);
        }

        setupManager.advanceInitialPlacement(state);
    }

    private void requireInitialPlacementPhase(GameState state) {
        if (state.getPhase() != GamePhase.INITIAL_PLACEMENT_FORWARD
                && state.getPhase() != GamePhase.INITIAL_PLACEMENT_REVERSE) {
            throw new InvalidPlacementException("Initial placement is only allowed during setup placement phases");
        }
    }

    private void requireSupply(AbstractPlayer player, BuildType buildType) {
        if (!player.getSupply().hasAvailable(buildType)) {
            throw new InvalidPlacementException("Player has no " + buildType + " supply available");
        }
    }

    private void giveStartingResources(GameState state, Board board, AbstractPlayer player, Intersection intersection) {
        for (String hexId : intersection.getAdjacentHexIds()) {
            HexTile hexTile = board.getHexTile(hexId)
                    .orElseThrow(() -> new InvalidPlacementException("Intersection references unknown hex: " + hexId));
            hexTile.getTerrainType().getProducedResource()
                    .ifPresent(resourceType -> giveStartingResource(state, player, resourceType, hexTile.getId()));
        }
    }

    private void giveStartingResource(GameState state, AbstractPlayer player, ResourceType resourceType, String hexTileId) {
        if (!state.getBank().hasResource(resourceType, 1)) {
            return;
        }

        state.getBank().giveResource(resourceType, 1);
        player.addResource(resourceType, 1);
        eventBus.publish(new ResourceProducedEvent(player.getId(), resourceType, 1, hexTileId));
        eventBus.publish(new ResourceChangedEvent(player.getId(), resourceType, 1, player.getResourceCount(resourceType)));
    }
}
