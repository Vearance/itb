package com.bananarepublic.event;

import com.bananarepublic.core.action.TradeAction;

import java.time.Instant;
import java.util.Objects;

public record TradeRejectedEvent(TradeAction tradeAction, String reason, Instant occurredAt) implements GameEvent {
    public TradeRejectedEvent(TradeAction tradeAction, String reason) {
        this(tradeAction, reason, Instant.now());
    }

    public TradeRejectedEvent {
        Objects.requireNonNull(tradeAction, "tradeAction");
        reason = reason == null || reason.isBlank() ? "Trade rejected" : reason;
        Objects.requireNonNull(occurredAt, "occurredAt");
    }
}
