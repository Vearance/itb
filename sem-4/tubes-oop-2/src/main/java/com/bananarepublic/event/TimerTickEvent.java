package com.bananarepublic.event;

import com.bananarepublic.core.player.PlayerId;

import java.time.Instant;
import java.util.Objects;

public record TimerTickEvent(PlayerId playerId, int turnNumber, int remainingSeconds, Instant occurredAt) implements GameEvent {
    public TimerTickEvent(PlayerId playerId, int turnNumber, int remainingSeconds) {
        this(playerId, turnNumber, remainingSeconds, Instant.now());
    }

    public TimerTickEvent {
        Objects.requireNonNull(playerId, "playerId");
        Objects.requireNonNull(occurredAt, "occurredAt");
        if (turnNumber < 0) {
            throw new IllegalArgumentException("Turn number cannot be negative");
        }
        if (remainingSeconds < 0) {
            throw new IllegalArgumentException("Remaining seconds cannot be negative");
        }
    }
}
