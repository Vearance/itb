package com.bananarepublic.core.player;

import com.bananarepublic.core.card.CardType;
import com.bananarepublic.core.card.ExperimentCard;
import com.bananarepublic.core.card.ExperimentCardId;
import com.bananarepublic.core.card.OwnedExperimentCard;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.Optional;

public final class PlayerCardHand {
    private final List<OwnedExperimentCard> cards = new ArrayList<>();

    public OwnedExperimentCard addCard(ExperimentCard card, int boughtTurnNumber) {
        OwnedExperimentCard ownedCard = new OwnedExperimentCard(card, boughtTurnNumber);
        cards.add(ownedCard);
        return ownedCard;
    }

    public Optional<OwnedExperimentCard> findCard(ExperimentCardId cardId) {
        Objects.requireNonNull(cardId, "cardId");
        return cards.stream()
                .filter(card -> card.getCardId().equals(cardId))
                .findFirst();
    }

    public OwnedExperimentCard removeCard(ExperimentCardId cardId) {
        OwnedExperimentCard ownedCard = findCard(cardId)
                .orElseThrow(() -> new IllegalArgumentException("Card is not in hand: " + cardId.value()));
        cards.remove(ownedCard);
        return ownedCard;
    }

    public int countByType(CardType type) {
        Objects.requireNonNull(type, "type");
        return (int) cards.stream()
                .filter(card -> card.getType() == type)
                .count();
    }

    public List<OwnedExperimentCard> getCards() {
        return List.copyOf(cards);
    }
}
