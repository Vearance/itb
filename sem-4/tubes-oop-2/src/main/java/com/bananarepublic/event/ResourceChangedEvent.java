package com.bananarepublic.event;

import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceType;

import java.time.Instant;

public record ResourceChangedEvent(PlayerId playerId, ResourceType resourceType, int delta, int newAmount, Instant occurredAt) implements GameEvent {
    public ResourceChangedEvent(PlayerId playerId, ResourceType resourceType, int delta, int newAmount) {
        this(playerId, resourceType, delta, newAmount, Instant.now());
    }
}
