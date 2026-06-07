package com.bananarepublic.event;

import com.bananarepublic.core.building.AbstractBuilding;

import java.time.Instant;

public record BuildingUpgradedEvent(
        AbstractBuilding previousBuilding,
        AbstractBuilding upgradedBuilding,
        String boardLocationId,
        Instant occurredAt
) implements GameEvent {
    public BuildingUpgradedEvent(AbstractBuilding previousBuilding, AbstractBuilding upgradedBuilding, String boardLocationId) {
        this(previousBuilding, upgradedBuilding, boardLocationId, Instant.now());
    }
}
