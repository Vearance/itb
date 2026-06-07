package com.bananarepublic.exception.card;

import com.bananarepublic.exception.GameException;

public final class DeckEmptyException extends GameException {
    public DeckEmptyException(String message) {
        super(message);
    }
}
