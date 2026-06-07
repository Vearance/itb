package com.bananarepublic.event;

@FunctionalInterface
public interface GameEventListener<T extends GameEvent> {
    void onEvent(T event);
}
