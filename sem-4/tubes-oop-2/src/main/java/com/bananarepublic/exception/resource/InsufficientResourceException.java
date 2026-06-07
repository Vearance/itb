package com.bananarepublic.exception.resource;

import com.bananarepublic.exception.GameException;

public class InsufficientResourceException extends GameException {
    public InsufficientResourceException(String message) {
        super(message);
    }
}
