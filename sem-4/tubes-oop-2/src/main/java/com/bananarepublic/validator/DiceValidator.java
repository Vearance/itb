package com.bananarepublic.validator;

public final class DiceValidator {
    private DiceValidator() {
    }

    public static void validateDieValue(int value) {
        if (value < 1 || value > 6) {
            throw new IllegalArgumentException("Dice value must be between 1 and 6");
        }
    }

    public static void validateDiceTotal(int diceTotal) {
        if (diceTotal < 2 || diceTotal > 12) {
            throw new IllegalArgumentException("Dice total must be between 2 and 12");
        }
    }
}
