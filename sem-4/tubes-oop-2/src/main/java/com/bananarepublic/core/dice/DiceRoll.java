package com.bananarepublic.core.dice;

import java.util.Objects;

public final class DiceRoll {
    private final Dice firstDice;
    private final Dice secondDice;

    public DiceRoll(Dice firstDice, Dice secondDice) {
        this.firstDice = Objects.requireNonNull(firstDice, "firstDice");
        this.secondDice = Objects.requireNonNull(secondDice, "secondDice");
    }

    public static DiceRoll of(int firstValue, int secondValue) {
        return new DiceRoll(new Dice(firstValue), new Dice(secondValue));
    }

    public Dice getFirstDice() {
        return firstDice;
    }

    public Dice getSecondDice() {
        return secondDice;
    }

    public int getTotal() {
        return firstDice.getValue() + secondDice.getValue();
    }

    // SIX SEVENNNNN
    public boolean isSeven() {
        return getTotal() == 7;
    }
}
