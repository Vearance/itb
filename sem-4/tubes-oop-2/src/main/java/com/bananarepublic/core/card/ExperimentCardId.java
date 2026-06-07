package com.bananarepublic.core.card;

public record ExperimentCardId(String value) {
    public ExperimentCardId {
        if (value == null || value.isBlank()) {
            throw new IllegalArgumentException("Card id cannot be blank");
        }
    }
}
