package com.bananarepublic.listener;

import com.bananarepublic.event.GameEventListener;
import com.bananarepublic.event.PluginLoadedEvent;

import java.util.Objects;
import java.util.function.Consumer;

public final class PluginListener implements GameEventListener<PluginLoadedEvent> {
    private final Consumer<PluginLoadedEvent> handler;

    public PluginListener(Consumer<PluginLoadedEvent> handler) {
        this.handler = Objects.requireNonNull(handler, "handler");
    }

    @Override
    public void onEvent(PluginLoadedEvent event) {
        handler.accept(event);
    }
}
