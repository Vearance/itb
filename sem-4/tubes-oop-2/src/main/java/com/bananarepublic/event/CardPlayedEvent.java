package com.bananarepublic.event;

import com.bananarepublic.core.card.CardType;
import com.bananarepublic.core.card.ExperimentCardId;
import com.bananarepublic.core.player.PlayerId;

import java.time.Instant;
import java.util.Objects;

public record CardPlayedEvent(PlayerId playerId, ExperimentCardId cardId, CardType cardType,
                              Instant occurredAt) implements GameEvent {
    public CardPlayedEvent(PlayerId playerId, ExperimentCardId cardId, CardType cardType) {
        this(playerId, cardId, cardType, Instant.now());
    }

    public CardPlayedEvent {
        Objects.requireNonNull(playerId, "playerId");
        Objects.requireNonNull(cardId, "cardId");
        Objects.requireNonNull(cardType, "cardType");
        Objects.requireNonNull(occurredAt, "occurredAt");
    }
}
