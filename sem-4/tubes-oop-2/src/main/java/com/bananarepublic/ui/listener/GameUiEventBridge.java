package com.bananarepublic.ui.listener;

import com.bananarepublic.event.GameEvent;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.GameEventListener;

import java.util.Objects;
import java.util.function.Consumer;

public final class GameUiEventBridge implements GameEventListener<GameEvent> {
    private volatile Consumer<GameEvent> eventHandler = event -> { };

    public static GameUiEventBridge registerTo(GameEventBus eventBus) {
        Objects.requireNonNull(eventBus, "eventBus");
        GameUiEventBridge bridge = new GameUiEventBridge();
        eventBus.register(GameEvent.class, bridge);
        return bridge;
    }

    private GameUiEventBridge() {
    }

    public void bind(Consumer<GameEvent> eventHandler) {
        this.eventHandler = Objects.requireNonNull(eventHandler, "eventHandler");
    }

    public void clear() {
        eventHandler = event -> { };
    }

    @Override
    public void onEvent(GameEvent event) {
        eventHandler.accept(event);
    }
}
