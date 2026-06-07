package com.bananarepublic.core.dice;

public final class ManualDiceRoller implements DiceRoller {
    private DiceRoll nextRoll;

    public ManualDiceRoller(int firstValue, int secondValue) {
        setNextRoll(firstValue, secondValue);
    }

    public void setNextRoll(int firstValue, int secondValue) {
        this.nextRoll = DiceRoll.of(firstValue, secondValue);
    }

    @Override
    public DiceRoll roll() {
        return nextRoll;
    }
}
