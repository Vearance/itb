package com.bananarepublic.event;

import com.bananarepublic.core.player.PlayerId;

import java.time.Instant;
import java.util.Objects;

public record TimerStartedEvent(PlayerId playerId, int turnNumber, int durationSeconds, Instant occurredAt) implements GameEvent {
    public TimerStartedEvent(PlayerId playerId, int turnNumber, int durationSeconds) {
        this(playerId, turnNumber, durationSeconds, Instant.now());
    }

    public TimerStartedEvent {
        Objects.requireNonNull(playerId, "playerId");
        Objects.requireNonNull(occurredAt, "occurredAt");
        if (turnNumber < 0) {
            throw new IllegalArgumentException("Turn number cannot be negative");
        }
        if (durationSeconds <= 0) {
            throw new IllegalArgumentException("Timer duration must be positive");
        }
    }
}
