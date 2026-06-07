package com.bananarepublic.core.resource;

import java.util.Objects;

public final class ResourceCard {
    private final ResourceType type;

    public ResourceCard(ResourceType type) {
        this.type = Objects.requireNonNull(type, "type");
    }

    public ResourceType getType() {
        return type;
    }
}
