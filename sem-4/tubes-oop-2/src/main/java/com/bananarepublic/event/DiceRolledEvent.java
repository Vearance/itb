package com.bananarepublic.event;

import com.bananarepublic.core.dice.DiceRoll;
import com.bananarepublic.core.player.PlayerId;

import java.time.Instant;

public record DiceRolledEvent(PlayerId playerId, DiceRoll diceRoll, Instant occurredAt) implements GameEvent {
    public DiceRolledEvent(PlayerId playerId, DiceRoll diceRoll) {
        this(playerId, diceRoll, Instant.now());
    }
}
