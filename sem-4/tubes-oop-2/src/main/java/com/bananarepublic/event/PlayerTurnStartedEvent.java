package com.bananarepublic.event;

import com.bananarepublic.core.player.PlayerId;

import java.time.Instant;

public record PlayerTurnStartedEvent(PlayerId playerId, int turnNumber, Instant occurredAt) implements GameEvent {
    public PlayerTurnStartedEvent(PlayerId playerId, int turnNumber) {
        this(playerId, turnNumber, Instant.now());
    }
}
