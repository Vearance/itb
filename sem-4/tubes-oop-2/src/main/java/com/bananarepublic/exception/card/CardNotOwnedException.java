package com.bananarepublic.exception.card;

public final class CardNotOwnedException extends InvalidCardPlayException {
    public CardNotOwnedException(String message) {
        super(message);
    }
}
