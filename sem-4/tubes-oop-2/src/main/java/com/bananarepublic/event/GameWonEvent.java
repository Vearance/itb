package com.bananarepublic.event;

import com.bananarepublic.core.player.PlayerId;

import java.time.Instant;

public record GameWonEvent(PlayerId winnerId, int victoryPoints, Instant occurredAt) implements GameEvent {
    public GameWonEvent(PlayerId winnerId, int victoryPoints) {
        this(winnerId, victoryPoints, Instant.now());
    }
}
