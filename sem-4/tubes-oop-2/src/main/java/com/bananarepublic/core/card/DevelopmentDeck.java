package com.bananarepublic.core.card;

import com.bananarepublic.exception.card.DeckEmptyException;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Deque;
import java.util.List;
import java.util.Objects;
import java.util.Random;

public final class DevelopmentDeck {
    private final Deque<ExperimentCard> cards;

    public DevelopmentDeck(List<ExperimentCard> cards) {
        this.cards = new ArrayDeque<>(Objects.requireNonNull(cards, "cards"));
    }

    public static DevelopmentDeck standardDeck() {
        return standardDeck(new Random());
    }

    public static DevelopmentDeck standardDeck(Random random) {
        Objects.requireNonNull(random, "random");
        List<ExperimentCard> cards = new ArrayList<>(CardFactory.createStandardCards());
        Collections.shuffle(cards, random);
        return new DevelopmentDeck(cards);
    }

    public ExperimentCard draw() {
        if (cards.isEmpty()) {
            throw new DeckEmptyException("Experiment card deck is empty");
        }
        int previousSize = cards.size();
        ExperimentCard drawnCard = cards.removeFirst();
        assert cards.size() == previousSize - 1 : "Drawing a card must reduce deck size by one";
        assert drawnCard != null : "Development deck cannot draw a null card";
        return drawnCard;
    }

    public void addCardToTop(ExperimentCard card) {
        cards.addFirst(Objects.requireNonNull(card, "card"));
    }

    public void addCardsToTop(List<ExperimentCard> newCards) {
        Objects.requireNonNull(newCards, "newCards");
        for (int i = newCards.size() - 1; i >= 0; i--) {
            addCardToTop(newCards.get(i));
        }
    }

    public int remainingCount() {
        return cards.size();
    }

    public boolean isEmpty() {
        return cards.isEmpty();
    }

    public List<ExperimentCard> snapshot() {
        return List.copyOf(cards);
    }
}
