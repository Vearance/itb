package com.bananarepublic.service;

import com.bananarepublic.core.dice.DiceRoll;
import com.bananarepublic.core.dice.DiceRoller;
import com.bananarepublic.core.game.GamePhase;
import com.bananarepublic.core.game.GameStatus;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.event.DiceRolledEvent;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.validator.TurnValidator;

import java.util.Objects;

public final class DiceService {
    private final DiceRoller diceRoller;
    private final GameEventBus eventBus;

    public DiceService(DiceRoller diceRoller, GameEventBus eventBus) {
        this.diceRoller = Objects.requireNonNull(diceRoller, "diceRoller");
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
    }

    public DiceRoll rollDice(GameState state) {
        Objects.requireNonNull(state, "state");
        TurnValidator.requireStatus(state, GameStatus.IN_PROGRESS);
        TurnValidator.requirePhase(state, GamePhase.ROLL_DICE);

        DiceRoll diceRoll = diceRoller.roll();
        eventBus.publish(new DiceRolledEvent(state.getCurrentPlayer().getId(), diceRoll));
        return diceRoll;
    }
}
