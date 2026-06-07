package com.bananarepublic.exception.rule;

import com.bananarepublic.exception.GameException;

public class InvalidPhaseException extends GameException {
    public InvalidPhaseException(String message) {
        super(message);
    }
    
}
