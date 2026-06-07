package com.bananarepublic.service;

import com.bananarepublic.config.BuildCostConfig;
import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.board.Intersection;
import com.bananarepublic.core.board.IntersectionId;
import com.bananarepublic.core.board.Path;
import com.bananarepublic.core.board.PathId;
import com.bananarepublic.core.building.AbstractBuilding;
import com.bananarepublic.core.building.BuildType;
import com.bananarepublic.core.building.BuildingFactory;
import com.bananarepublic.core.building.Pipe;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.BuildingPlacedEvent;
import com.bananarepublic.event.BuildingUpgradedEvent;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.ResourceChangedEvent;
import com.bananarepublic.validator.BuildValidator;

import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

public final class BuildService {
    private final BuildValidator buildValidator;
    private final LongestRoadService longestRoadService;
    private final GameEventBus eventBus;

    public BuildService(GameEventBus eventBus) {
        this(new BuildValidator(), new LongestRoadService(), eventBus);
    }

    public BuildService(BuildValidator buildValidator, GameEventBus eventBus) {
        this(buildValidator, new LongestRoadService(), eventBus);
    }

    public BuildService(BuildValidator buildValidator, LongestRoadService longestRoadService, GameEventBus eventBus) {
        this.buildValidator = Objects.requireNonNull(buildValidator, "buildValidator");
        this.longestRoadService = Objects.requireNonNull(longestRoadService, "longestRoadService");
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
    }

    public void buildPipe(GameState state, Board board, PathId pathId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(pathId, "pathId");

        AbstractPlayer player = state.getCurrentPlayer();
        ResourceBundle cost = BuildCostConfig.getCost(BuildType.PIPE).getResources();
        buildValidator.validateBuildPhase(state);
        buildValidator.validateHasSupply(player, BuildType.PIPE);
        buildValidator.validateCanAfford(player, cost);
        buildValidator.validatePipe(board, pathId, player.getId());

        Path path = buildValidator.requirePath(board, pathId);
        Pipe pipe = (Pipe) BuildingFactory.create(BuildType.PIPE, player.getId());
        spendBuildCost(state, player, cost);
        player.getSupply().use(BuildType.PIPE);
        path.placePipe(pipe);
        refreshLongestRoad(board, state);

        eventBus.publish(new BuildingPlacedEvent(pipe, pathId.value()));
    }

    public void buildFreePipe(GameState state, Board board, PathId pathId) {
        buildFreePipes(state, board, List.of(pathId));
    }

    public void buildFreePipes(GameState state, Board board, List<PathId> pathIds) {
        validateCanBuildFreePipes(state, board, pathIds);

        AbstractPlayer player = state.getCurrentPlayer();
        for (PathId pathId : pathIds) {
            Path path = buildValidator.requirePath(board, pathId);
            Pipe pipe = (Pipe) BuildingFactory.create(BuildType.PIPE, player.getId());
            player.getSupply().use(BuildType.PIPE);
            path.placePipe(pipe);

            eventBus.publish(new BuildingPlacedEvent(pipe, pathId.value()));
        }
        refreshLongestRoad(board, state);
    }

    public void validateCanBuildFreePipe(GameState state, Board board, PathId pathId) {
        validateCanBuildFreePipes(state, board, List.of(pathId));
    }

    public void validateCanBuildFreePipes(GameState state, Board board, List<PathId> pathIds) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(pathIds, "pathIds");
        if (pathIds.isEmpty()) {
            throw new IllegalArgumentException("Minimal satu path Pipa gratis harus dipilih");
        }

        AbstractPlayer player = state.getCurrentPlayer();
        buildValidator.validateBuildPhase(state);
        if (player.getSupply().getPipes() < pathIds.size()) {
            throw new IllegalStateException("Supply Pipa tidak cukup untuk Pipa gratis");
        }

        Set<PathId> uniquePathIds = new HashSet<>(pathIds);
        if (uniquePathIds.size() != pathIds.size()) {
            throw new IllegalArgumentException("Path Pipa gratis harus berbeda");
        }

        for (PathId pathId : pathIds) {
            buildValidator.validatePipe(board, pathId, player.getId());
        }
    }

    public void buildSettlement(GameState state, Board board, IntersectionId intersectionId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(intersectionId, "intersectionId");

        AbstractPlayer player = state.getCurrentPlayer();
        ResourceBundle cost = BuildCostConfig.getCost(BuildType.SETTLEMENT).getResources();
        buildValidator.validateBuildPhase(state);
        buildValidator.validateHasSupply(player, BuildType.SETTLEMENT);
        buildValidator.validateCanAfford(player, cost);
        buildValidator.validateSettlement(board, intersectionId, player.getId());

        Intersection intersection = buildValidator.requireIntersection(board, intersectionId);
        AbstractBuilding settlement = BuildingFactory.create(BuildType.SETTLEMENT, player.getId());
        spendBuildCost(state, player, cost);
        player.getSupply().use(BuildType.SETTLEMENT);
        player.getScore().addPublicPoints(settlement.getVictoryPoints());
        intersection.placeBuilding(settlement);

        assert intersection.getBuilding()
                .map(building -> building.getOwnerId().equals(player.getId())
                        && building.getBuildType() == BuildType.SETTLEMENT)
                .orElse(false) : "Settlement build must place the current player's settlement";
        refreshLongestRoad(board, state);
        eventBus.publish(new BuildingPlacedEvent(settlement, intersectionId.value()));
    }

    public void upgradeSettlementToLaboratory(GameState state, Board board, IntersectionId intersectionId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(intersectionId, "intersectionId");

        AbstractPlayer player = state.getCurrentPlayer();
        ResourceBundle cost = BuildCostConfig.getCost(BuildType.LABORATORY).getResources();
        buildValidator.validateBuildPhase(state);
        buildValidator.validateHasSupply(player, BuildType.LABORATORY);
        buildValidator.validateCanAfford(player, cost);
        buildValidator.validateLaboratoryUpgrade(board, intersectionId, player.getId());

        Intersection intersection = buildValidator.requireIntersection(board, intersectionId);
        AbstractBuilding previousBuilding = intersection.getBuilding().orElseThrow();
        AbstractBuilding laboratory = BuildingFactory.create(BuildType.LABORATORY, player.getId());
        spendBuildCost(state, player, cost);
        player.getSupply().use(BuildType.LABORATORY);
        player.getSupply().returnSettlement();
        player.getScore().addPublicPoints(laboratory.getVictoryPoints() - previousBuilding.getVictoryPoints());
        intersection.upgradeBuilding(laboratory);

        eventBus.publish(new BuildingUpgradedEvent(previousBuilding, laboratory, intersectionId.value()));
    }

    private void spendBuildCost(GameState state, AbstractPlayer player, ResourceBundle cost) {
        player.getInventory().spendResourceBundle(cost);
        state.getBank().receiveResourceBundle(cost);
        for (ResourceType type : ResourceType.values()) {
            int amount = cost.get(type);
            if (amount == 0) {
                continue;
            }
            eventBus.publish(new ResourceChangedEvent(player.getId(), type, -amount, player.getResourceCount(type)));
        }
    }

    private void refreshLongestRoad(Board board, GameState state) {
        longestRoadService.updateAllRoadLengths(board, state);
    }
}
