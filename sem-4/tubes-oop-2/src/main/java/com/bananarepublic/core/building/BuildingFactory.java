package com.bananarepublic.core.building;

import com.bananarepublic.core.player.PlayerId;

public final class BuildingFactory {
    private BuildingFactory() {
    }

    public static AbstractBuilding create(BuildType buildType, PlayerId ownerId) {
        return switch (buildType) {
            case PIPE -> new Pipe(ownerId);
            case SETTLEMENT -> new Settlement(ownerId);
            case LABORATORY -> new Laboratory(ownerId);
        };
    }
}
