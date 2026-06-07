package com.bananarepublic.core.player;

import com.bananarepublic.core.resource.ResourceType;

public interface Player {
    String getName();
    int getResourceCount(ResourceType type);
    void addResource(ResourceType type, int amount);
    void removeResource(ResourceType type, int amount);
}
