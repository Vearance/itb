package com.bananarepublic.ui.controller;

import com.bananarepublic.core.card.ExperimentCardId;

import java.util.Objects;

public record PendingKnightCardSelection(ExperimentCardId cardId) {
    public PendingKnightCardSelection {
        Objects.requireNonNull(cardId, "cardId");
    }
}
