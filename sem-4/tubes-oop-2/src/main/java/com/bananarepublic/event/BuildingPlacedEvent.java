package com.bananarepublic.event;

import com.bananarepublic.core.building.AbstractBuilding;

import java.time.Instant;

public record BuildingPlacedEvent(AbstractBuilding building, String boardLocationId, Instant occurredAt) implements GameEvent {
    public BuildingPlacedEvent(AbstractBuilding building, String boardLocationId) {
        this(building, boardLocationId, Instant.now());
    }
}
