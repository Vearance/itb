package com.bananarepublic.event;

import com.bananarepublic.core.player.PlayerId;

import java.time.Instant;
import java.util.Objects;

public record TimerExpiredEvent(PlayerId playerId, int turnNumber, Instant occurredAt) implements GameEvent {
    public TimerExpiredEvent(PlayerId playerId, int turnNumber) {
        this(playerId, turnNumber, Instant.now());
    }

    public TimerExpiredEvent {
        Objects.requireNonNull(playerId, "playerId");
        Objects.requireNonNull(occurredAt, "occurredAt");
        if (turnNumber < 0) {
            throw new IllegalArgumentException("Turn number cannot be negative");
        }
    }
}
