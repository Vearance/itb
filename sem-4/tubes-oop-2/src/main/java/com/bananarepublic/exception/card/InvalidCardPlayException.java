package com.bananarepublic.exception.card;

import com.bananarepublic.exception.GameException;

public class InvalidCardPlayException extends GameException {
    public InvalidCardPlayException(String message) {
        super(message);
    }
}
