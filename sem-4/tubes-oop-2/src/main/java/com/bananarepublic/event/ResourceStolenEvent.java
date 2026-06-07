package com.bananarepublic.event;

import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceType;

import java.time.Instant;
import java.util.Objects;

public record ResourceStolenEvent(PlayerId thiefId, PlayerId victimId, ResourceType resourceType,
                                  Instant occurredAt) implements GameEvent {
    public ResourceStolenEvent(PlayerId thiefId, PlayerId victimId, ResourceType resourceType) {
        this(thiefId, victimId, resourceType, Instant.now());
    }

    public ResourceStolenEvent {
        Objects.requireNonNull(thiefId, "thiefId");
        Objects.requireNonNull(victimId, "victimId");
        Objects.requireNonNull(resourceType, "resourceType");
        Objects.requireNonNull(occurredAt, "occurredAt");
    }
}
