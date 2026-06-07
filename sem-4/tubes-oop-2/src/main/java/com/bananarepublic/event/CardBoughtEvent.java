package com.bananarepublic.event;

import com.bananarepublic.core.card.CardType;
import com.bananarepublic.core.card.ExperimentCardId;
import com.bananarepublic.core.player.PlayerId;

import java.time.Instant;
import java.util.Objects;

public record CardBoughtEvent(PlayerId playerId, ExperimentCardId cardId, CardType cardType, int remainingDeckSize,
                              Instant occurredAt) implements GameEvent {
    public CardBoughtEvent(PlayerId playerId, ExperimentCardId cardId, CardType cardType, int remainingDeckSize) {
        this(playerId, cardId, cardType, remainingDeckSize, Instant.now());
    }

    public CardBoughtEvent {
        Objects.requireNonNull(playerId, "playerId");
        Objects.requireNonNull(cardId, "cardId");
        Objects.requireNonNull(cardType, "cardType");
        Objects.requireNonNull(occurredAt, "occurredAt");
        if (remainingDeckSize < 0) {
            throw new IllegalArgumentException("Remaining deck size cannot be negative");
        }
    }
}
