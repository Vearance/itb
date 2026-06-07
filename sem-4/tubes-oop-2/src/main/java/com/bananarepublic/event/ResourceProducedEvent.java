package com.bananarepublic.event;

import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceType;

import java.time.Instant;

public record ResourceProducedEvent(PlayerId playerId, ResourceType resourceType, int amount, String hexTileId, Instant occurredAt) implements GameEvent {
    public ResourceProducedEvent(PlayerId playerId, ResourceType resourceType, int amount, String hexTileId) {
        this(playerId, resourceType, amount, hexTileId, Instant.now());
    }
}
