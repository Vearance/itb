package com.bananarepublic.core.card;

public final class MonopolyCard extends AbstractExperimentCard {
    public MonopolyCard(ExperimentCardId id) {
        super(id, CardType.MONOPOLY, "Monopoly", "Choose one resource type and take all of it from every other player.");
    }
}
