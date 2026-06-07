package com.bananarepublic.event;

import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;

import java.time.Instant;
import java.util.Objects;
import java.util.Optional;

public record TradeCompletedEvent(
        PlayerId activePlayerId,
        Optional<PlayerId> partnerPlayerId,
        ResourceBundle givenResources,
        ResourceBundle receivedResources,
        boolean harborTrade,
        Instant occurredAt
) implements GameEvent {
    public TradeCompletedEvent(
            PlayerId activePlayerId,
            PlayerId partnerPlayerId,
            ResourceBundle givenResources,
            ResourceBundle receivedResources,
            boolean harborTrade
    ) {
        this(activePlayerId, Optional.ofNullable(partnerPlayerId), givenResources, receivedResources, harborTrade, Instant.now());
    }

    public TradeCompletedEvent {
        Objects.requireNonNull(activePlayerId, "activePlayerId");
        Objects.requireNonNull(partnerPlayerId, "partnerPlayerId");
        givenResources = Objects.requireNonNull(givenResources, "givenResources").copy();
        receivedResources = Objects.requireNonNull(receivedResources, "receivedResources").copy();
        Objects.requireNonNull(occurredAt, "occurredAt");
    }

    @Override
    public ResourceBundle givenResources() {
        return givenResources.copy();
    }

    @Override
    public ResourceBundle receivedResources() {
        return receivedResources.copy();
    }
}
