package com.bananarepublic.validator;

import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.board.Intersection;
import com.bananarepublic.core.board.IntersectionId;
import com.bananarepublic.core.board.Path;
import com.bananarepublic.core.board.PathId;
import com.bananarepublic.core.building.BuildType;
import com.bananarepublic.core.game.GamePhase;
import com.bananarepublic.core.game.GameStatus;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.exception.build.BuildingLimitExceededException;
import com.bananarepublic.exception.build.IntersectionOccupiedException;
import com.bananarepublic.exception.build.InvalidBuildException;
import com.bananarepublic.exception.build.InvalidPlacementException;
import com.bananarepublic.exception.build.PathOccupiedException;
import com.bananarepublic.exception.resource.InsufficientResourceException;

import java.util.Objects;

public final class BuildValidator {
    public void validateBuildPhase(GameState state) {
        Objects.requireNonNull(state, "state");
        if (state.getStatus() != GameStatus.IN_PROGRESS) {
            throw new InvalidBuildException("Pembangunan hanya bisa dilakukan saat permainan berlangsung");
        }
        if (state.getPhase() != GamePhase.PLAYER_ACTIONS) {
            throw new InvalidBuildException("Pembangunan hanya bisa dilakukan pada fase aksi pemain");
        }
    }

    public void validateCanAfford(AbstractPlayer player, ResourceBundle cost) {
        Objects.requireNonNull(player, "player");
        Objects.requireNonNull(cost, "cost");
        if (!player.getInventory().hasResources(cost)) {
            throw new InsufficientResourceException("Resource pemain tidak cukup untuk membangun");
        }
    }

    public void validateHasSupply(AbstractPlayer player, BuildType buildType) {
        Objects.requireNonNull(player, "player");
        Objects.requireNonNull(buildType, "buildType");
        if (!player.getSupply().hasAvailable(buildType)) {
            throw new BuildingLimitExceededException("Supply " + displayBuildType(buildType) + " sudah habis");
        }
    }

    public void validatePipe(Board board, PathId pathId, PlayerId playerId) {
        Path path = requirePath(board, pathId);
        if (path.hasPipe()) {
            throw new PathOccupiedException(pathId);
        }
        if (!isConnectedToOwnedNetwork(board, path, playerId)) {
            throw new InvalidPlacementException("Pipa harus tersambung ke Pipa, Pos, atau Lab milik pemain");
        }
    }

    public void validateSettlement(Board board, IntersectionId intersectionId, PlayerId playerId) {
        validateInitialSettlement(board, intersectionId);
        if (!hasOwnedPipeTouching(board, intersectionId, playerId)) {
            throw new InvalidPlacementException("Pos Pantau harus tersambung ke minimal satu Pipa milik pemain");
        }
    }

    public void validateLaboratoryUpgrade(Board board, IntersectionId intersectionId, PlayerId playerId) {
        Intersection intersection = requireIntersection(board, intersectionId);
        if (intersection.getBuilding().isEmpty()) {
            throw new InvalidPlacementException("Lab harus upgrade dari Pos Pantau");
        }
        var building = intersection.getBuilding().orElseThrow();
        if (!building.getOwnerId().equals(playerId)) {
            throw new InvalidPlacementException("Lab hanya bisa upgrade dari Pos Pantau milik pemain aktif");
        }
        if (building.getBuildType() != BuildType.SETTLEMENT) {
            throw new InvalidPlacementException("Lab hanya bisa upgrade dari Pos Pantau, bukan bangunan lain");
        }
    }

    public void validateInitialSettlement(Board board, IntersectionId intersectionId) {
        Intersection intersection = requireIntersection(board, intersectionId);
        if (intersection.hasBuilding()) {
            throw new IntersectionOccupiedException(intersectionId);
        }
        requireDistanceRule(board, intersectionId);
    }

    public void validateInitialPipe(Board board, PathId pathId, IntersectionId settlementIntersectionId) {
        Path path = requirePath(board, pathId);
        if (path.hasPipe()) {
            throw new PathOccupiedException(pathId);
        }
        if (!path.touches(settlementIntersectionId)) {
            throw new InvalidPlacementException("Pipa awal harus menempel pada Pos Pantau yang baru dipasang");
        }
    }

    public Intersection requireIntersection(Board board, IntersectionId intersectionId) {
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(intersectionId, "intersectionId");
        Intersection intersection = board.getIntersections().get(intersectionId);
        if (intersection == null) {
            throw new InvalidPlacementException("Intersection tidak ditemukan: " + intersectionId.value());
        }
        return intersection;
    }

    public Path requirePath(Board board, PathId pathId) {
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(pathId, "pathId");
        Path path = board.getPaths().get(pathId);
        if (path == null) {
            throw new InvalidPlacementException("Path tidak ditemukan: " + pathId.value());
        }
        return path;
    }

    private void requireDistanceRule(Board board, IntersectionId intersectionId) {
        for (Path path : board.getPaths().values()) {
            if (!path.touches(intersectionId)) {
                continue;
            }

            IntersectionId neighborId = otherIntersectionId(path, intersectionId);
            Intersection neighbor = board.getIntersections().get(neighborId);
            if (neighbor != null && neighbor.hasBuilding()) {
                throw new InvalidPlacementException(
                        "Distance rule dilanggar: terlalu dekat dengan Pos/Lab lain"
                );
            }
        }
    }

    private String displayBuildType(BuildType buildType) {
        return switch (buildType) {
            case PIPE -> "Pipa";
            case SETTLEMENT -> "Pos Pantau";
            case LABORATORY -> "Laboratorium";
        };
    }

    private IntersectionId otherIntersectionId(Path path, IntersectionId intersectionId) {
        if (path.getFirstIntersectionId().equals(intersectionId)) {
            return path.getSecondIntersectionId();
        }
        return path.getFirstIntersectionId();
    }

    private boolean isConnectedToOwnedNetwork(Board board, Path path, PlayerId playerId) {
        return isConnectedAtEndpoint(board, path.getFirstIntersectionId(), playerId)
                || isConnectedAtEndpoint(board, path.getSecondIntersectionId(), playerId);
    }

    private boolean isConnectedAtEndpoint(Board board, IntersectionId endpointId, PlayerId playerId) {
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
        return !blockedByOpponentBuilding && hasOwnedPipeTouching(board, endpointId, playerId);
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
}
