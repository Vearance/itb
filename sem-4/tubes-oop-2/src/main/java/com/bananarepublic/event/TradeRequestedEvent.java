package com.bananarepublic.event;

import com.bananarepublic.core.action.TradeAction;

import java.time.Instant;
import java.util.Objects;

public record TradeRequestedEvent(TradeAction tradeAction, Instant occurredAt) implements GameEvent {
    public TradeRequestedEvent(TradeAction tradeAction) {
        this(tradeAction, Instant.now());
    }

    public TradeRequestedEvent {
        Objects.requireNonNull(tradeAction, "tradeAction");
        Objects.requireNonNull(occurredAt, "occurredAt");
    }
}
