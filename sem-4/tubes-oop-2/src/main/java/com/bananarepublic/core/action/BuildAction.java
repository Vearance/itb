package com.bananarepublic.core.action;

import com.bananarepublic.core.board.IntersectionId;
import com.bananarepublic.core.board.PathId;
import com.bananarepublic.core.building.BuildType;
import com.bananarepublic.core.player.PlayerId;

import java.util.Objects;
import java.util.Optional;

public final class BuildAction {
    private final PlayerId playerId;
    private final BuildType buildType;
    private final IntersectionId intersectionId;
    private final PathId pathId;

    private BuildAction(PlayerId playerId, BuildType buildType, IntersectionId intersectionId, PathId pathId) {
        this.playerId = Objects.requireNonNull(playerId, "playerId");
        this.buildType = Objects.requireNonNull(buildType, "buildType");
        this.intersectionId = intersectionId;
        this.pathId = pathId;
    }

    public static BuildAction pipe(PlayerId playerId, PathId pathId) {
        return new BuildAction(playerId, BuildType.PIPE, null, Objects.requireNonNull(pathId, "pathId"));
    }

    public static BuildAction settlement(PlayerId playerId, IntersectionId intersectionId) {
        return new BuildAction(playerId, BuildType.SETTLEMENT, Objects.requireNonNull(intersectionId, "intersectionId"), null);
    }

    public static BuildAction laboratory(PlayerId playerId, IntersectionId intersectionId) {
        return new BuildAction(playerId, BuildType.LABORATORY, Objects.requireNonNull(intersectionId, "intersectionId"), null);
    }

    public PlayerId getPlayerId() {
        return playerId;
    }

    public BuildType getBuildType() {
        return buildType;
    }

    public Optional<IntersectionId> getIntersectionId() {
        return Optional.ofNullable(intersectionId);
    }

    public Optional<PathId> getPathId() {
        return Optional.ofNullable(pathId);
    }
}
