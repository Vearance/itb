package com.bananarepublic.event;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;

public final class GameEventBus {
    private final Map<Class<? extends GameEvent>, List<GameEventListener<? extends GameEvent>>> listeners = new ConcurrentHashMap<>();

    public <T extends GameEvent> void register(Class<T> eventType, GameEventListener<? super T> listener) {
        Objects.requireNonNull(eventType, "eventType");
        Objects.requireNonNull(listener, "listener");
        listeners.computeIfAbsent(eventType, ignored -> new ArrayList<>())
                .add(castListener(listener));
    }
    public void publish(GameEvent event) {
        Objects.requireNonNull(event, "event");
        listeners.forEach((registeredType, registeredListeners) -> {
            if (registeredType.isAssignableFrom(event.getClass())) {
                List.copyOf(registeredListeners).forEach(listener -> notifyListener(listener, event));
            }
        });
    }

    public int listenerCount(Class<? extends GameEvent> eventType) {
        return listeners.getOrDefault(eventType, List.of()).size();
    }

    @SuppressWarnings("unchecked")
    private <T extends GameEvent> GameEventListener<? extends GameEvent> castListener(GameEventListener<? super T> listener) {
        return (GameEventListener<? extends GameEvent>) listener;
    }
    @SuppressWarnings("unchecked")
    private <T extends GameEvent> void notifyListener(GameEventListener<T> listener, GameEvent event) {
        listener.onEvent((T) event);
    }
}
