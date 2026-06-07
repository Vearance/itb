package com.bananarepublic.listener;

import com.bananarepublic.event.GameEventListener;
import com.bananarepublic.event.GameWonEvent;

import java.util.Objects;
import java.util.function.Consumer;

public final class GameResultListener implements GameEventListener<GameWonEvent> {
    private final Consumer<GameWonEvent> handler;

    public GameResultListener(Consumer<GameWonEvent> handler) {
        this.handler = Objects.requireNonNull(handler, "handler");
    }

    @Override
    public void onEvent(GameWonEvent event) {
        handler.accept(event);
    }
}
