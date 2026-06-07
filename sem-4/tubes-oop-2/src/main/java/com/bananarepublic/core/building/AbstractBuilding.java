package com.bananarepublic.core.building;

import com.bananarepublic.core.player.PlayerId;

import java.util.Objects;

public abstract class AbstractBuilding {
    private final PlayerId ownerId;
    private final BuildType buildType;
    private final int victoryPoints;

    protected AbstractBuilding(PlayerId ownerId, BuildType buildType, int victoryPoints) {
        this.ownerId = Objects.requireNonNull(ownerId, "ownerId");
        this.buildType = Objects.requireNonNull(buildType, "buildType");
        this.victoryPoints = victoryPoints;
    }

    public PlayerId getOwnerId() {
        return ownerId;
    }

    public BuildType getBuildType() {
        return buildType;
    }

    public int getVictoryPoints() {
        return victoryPoints;
    }
}
