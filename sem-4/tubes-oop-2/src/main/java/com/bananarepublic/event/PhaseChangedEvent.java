package com.bananarepublic.event;

import java.time.Instant;

import com.bananarepublic.core.game.GamePhase;

public record PhaseChangedEvent(GamePhase previousPhase, GamePhase currentPhase, Instant occurredAt) implements GameEvent {
    public PhaseChangedEvent(GamePhase previousPhase, GamePhase currentPhase) {
        this(previousPhase, currentPhase, Instant.now());
    }
}
