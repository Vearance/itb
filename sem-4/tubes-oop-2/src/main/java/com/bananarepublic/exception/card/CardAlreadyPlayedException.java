package com.bananarepublic.exception.card;

public final class CardAlreadyPlayedException extends InvalidCardPlayException {
    public CardAlreadyPlayedException(String message) {
        super(message);
    }
}
