package com.bananarepublic.exception.rule;

import com.bananarepublic.exception.GameException;

public class VictoryConditionException extends GameException {
    public VictoryConditionException(String message) {
        super(message);
    }
}
