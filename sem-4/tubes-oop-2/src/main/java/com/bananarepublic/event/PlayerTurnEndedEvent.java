package com.bananarepublic.event;

import com.bananarepublic.core.player.PlayerId;

import java.time.Instant;

public record PlayerTurnEndedEvent(PlayerId playerId, int turnNumber, Instant occurredAt) implements GameEvent {
    public PlayerTurnEndedEvent(PlayerId playerId, int turnNumber) {
        this(playerId, turnNumber, Instant.now());
    }
}
