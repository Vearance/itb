package com.bananarepublic.core.card;

import java.util.Objects;

public final class OwnedExperimentCard {
    private final ExperimentCard card;
    private final int boughtTurnNumber;

    public OwnedExperimentCard(ExperimentCard card, int boughtTurnNumber) {
        if (boughtTurnNumber < 0) {
            throw new IllegalArgumentException("Bought turn number cannot be negative");
        }
        this.card = Objects.requireNonNull(card, "card");
        this.boughtTurnNumber = boughtTurnNumber;
    }

    public ExperimentCard getCard() {
        return card;
    }

    public ExperimentCardId getCardId() {
        return card.getId();
    }

    public CardType getType() {
        return card.getType();
    }

    public int getBoughtTurnNumber() {
        return boughtTurnNumber;
    }
}
