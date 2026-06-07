package com.bananarepublic.service;

import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.PlayerId;

import java.util.Objects;

public final class VictoryPointService {
    public void addHiddenVictoryPoint(GameState state, PlayerId playerId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(playerId, "playerId");
        state.getPlayerById(playerId).getScore().addHiddenPoints(1);
    }
}
