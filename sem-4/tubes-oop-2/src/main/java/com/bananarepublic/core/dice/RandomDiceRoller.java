package com.bananarepublic.core.dice;

import java.util.Objects;
import java.util.Random;

public final class RandomDiceRoller implements DiceRoller {
    private final Random random;

    public RandomDiceRoller() {
        this(new Random());
    }

    public RandomDiceRoller(Random random) {
        this.random = Objects.requireNonNull(random, "random");
    }

    @Override
    public DiceRoll roll() {
        return DiceRoll.of(rollDie(), rollDie());
    }

    private int rollDie() {
        return random.nextInt(6) + 1;
    }
}
