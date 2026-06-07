package com.bananarepublic.event;

import com.bananarepublic.core.player.PlayerId;

import java.time.Instant;
import java.util.Objects;

public record RobberMovedEvent(PlayerId playerId, String previousHexTileId, String newHexTileId,
                               Instant occurredAt) implements GameEvent {
    public RobberMovedEvent(PlayerId playerId, String previousHexTileId, String newHexTileId) {
        this(playerId, previousHexTileId, newHexTileId, Instant.now());
    }

    public RobberMovedEvent {
        Objects.requireNonNull(playerId, "playerId");
        if (previousHexTileId == null || previousHexTileId.isBlank()) {
            throw new IllegalArgumentException("Previous hex tile id cannot be blank");
        }
        if (newHexTileId == null || newHexTileId.isBlank()) {
            throw new IllegalArgumentException("New hex tile id cannot be blank");
        }
        Objects.requireNonNull(occurredAt, "occurredAt");
    }
}
