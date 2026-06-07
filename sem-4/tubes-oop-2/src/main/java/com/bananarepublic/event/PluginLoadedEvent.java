package com.bananarepublic.event;

import com.bananarepublic.plugin.PluginMetadata;

import java.time.Instant;
import java.util.List;
import java.util.Objects;

public record PluginLoadedEvent(String sourcePath, List<PluginMetadata> plugins, Instant occurredAt) implements GameEvent {
    public PluginLoadedEvent(String sourcePath, List<PluginMetadata> plugins) {
        this(sourcePath, plugins, Instant.now());
    }

    public PluginLoadedEvent {
        if (sourcePath == null || sourcePath.isBlank()) {
            throw new IllegalArgumentException("Plugin source path cannot be blank");
        }
        plugins = List.copyOf(Objects.requireNonNull(plugins, "plugins"));
        occurredAt = Objects.requireNonNull(occurredAt, "occurredAt");
    }
}
