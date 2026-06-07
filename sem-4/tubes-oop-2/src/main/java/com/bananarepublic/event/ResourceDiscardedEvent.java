package com.bananarepublic.event;

import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;

import java.time.Instant;
import java.util.Objects;

public record ResourceDiscardedEvent(PlayerId playerId, ResourceBundle discardedResources, int remainingResourceCount,
                                     Instant occurredAt) implements GameEvent {
    public ResourceDiscardedEvent(PlayerId playerId, ResourceBundle discardedResources, int remainingResourceCount) {
        this(playerId, discardedResources, remainingResourceCount, Instant.now());
    }

    public ResourceDiscardedEvent {
        Objects.requireNonNull(playerId, "playerId");
        discardedResources = Objects.requireNonNull(discardedResources, "discardedResources").copy();
        Objects.requireNonNull(occurredAt, "occurredAt");
        if (remainingResourceCount < 0) {
            throw new IllegalArgumentException("Remaining resource count cannot be negative");
        }
    }

    @Override
    public ResourceBundle discardedResources() {
        return discardedResources.copy();
    }
}
