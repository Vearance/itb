package com.bananarepublic.plugin;

import java.time.Instant;
import java.util.Objects;

public record PluginMetadata(
        String id,
        String name,
        String description,
        String className,
        String sourcePath,
        Instant loadedAt
) {
    public PluginMetadata {
        if (id == null || id.isBlank()) {
            throw new IllegalArgumentException("Plugin id cannot be blank");
        }
        if (name == null || name.isBlank()) {
            throw new IllegalArgumentException("Plugin name cannot be blank");
        }
        if (description == null || description.isBlank()) {
            throw new IllegalArgumentException("Plugin description cannot be blank");
        }
        if (className == null || className.isBlank()) {
            throw new IllegalArgumentException("Plugin class name cannot be blank");
        }
        if (sourcePath == null || sourcePath.isBlank()) {
            throw new IllegalArgumentException("Plugin source path cannot be blank");
        }
        loadedAt = Objects.requireNonNull(loadedAt, "loadedAt");
    }
}
