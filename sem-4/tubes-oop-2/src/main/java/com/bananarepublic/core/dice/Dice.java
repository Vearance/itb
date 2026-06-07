package com.bananarepublic.core.dice;

import com.bananarepublic.validator.DiceValidator;

public final class Dice {
    private final int value;

    public Dice(int value) {
        DiceValidator.validateDieValue(value);
        this.value = value;
    }

    public int getValue() {
        return value;
    }
}
