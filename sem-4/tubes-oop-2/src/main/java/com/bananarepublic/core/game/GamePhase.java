package com.bananarepublic.core.game;

public enum GamePhase {
    SETUP, // player selection, etc.
    DETERMINE_START_PLAYER, // rolling the first dice
    INITIAL_PLACEMENT_FORWARD, // CW, place a house and a pipe
    INITIAL_PLACEMENT_REVERSE, // CCW, place a house (get resource) and a pipe
    ROLL_DICE, // common roll dice phase
    DISCARD_RESOURCES, // before moving robber, every player w/ > 7 resources need to discard 50%
    MOVE_ROBBER, // usecard or roles 7
    PLAYER_ACTIONS, // trade, build, or play development cards
    END_TURN, // when player pressed end turn; add game checker, etc.
}
