package com.bananarepublic.exception.build;

import com.bananarepublic.exception.GameException;

public class InvalidBuildException extends GameException {
    public InvalidBuildException(String message) {
        super(message);
    }
}
