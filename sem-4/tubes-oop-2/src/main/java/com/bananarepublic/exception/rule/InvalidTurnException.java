package com.bananarepublic.exception.rule;

import com.bananarepublic.exception.GameException;

public class InvalidTurnException extends GameException {

        public InvalidTurnException(String message) {
            super(message);
        }
}
