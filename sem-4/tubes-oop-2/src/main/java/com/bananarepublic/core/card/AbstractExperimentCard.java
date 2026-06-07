package com.bananarepublic.core.card;

import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.Player;
import com.bananarepublic.exception.card.InvalidCardPlayException;

import java.util.Objects;

public abstract class AbstractExperimentCard implements ExperimentCard {
    private final ExperimentCardId id;
    private final CardType type;
    private final String cardName;
    private final String description;

    protected AbstractExperimentCard(ExperimentCardId id, CardType type, String cardName, String description) {
        if (cardName == null || cardName.isBlank()) {
            throw new IllegalArgumentException("Card name cannot be blank");
        }
        if (description == null || description.isBlank()) {
            throw new IllegalArgumentException("Card description cannot be blank");
        }
        this.id = Objects.requireNonNull(id, "id");
        this.type = Objects.requireNonNull(type, "type");
        this.cardName = cardName;
        this.description = description;
    }

    @Override
    public ExperimentCardId getId() {
        return id;
    }

    @Override
    public CardType getType() {
        return type;
    }

    @Override
    public String getCardName() {
        return cardName;
    }

    @Override
    public String getDescription() {
        return description;
    }

    @Override
    public void applyEffect(GameState state, Player player) {
        throw new InvalidCardPlayException(cardName + " uses a built-in card service and cannot be applied directly");
    }
}
