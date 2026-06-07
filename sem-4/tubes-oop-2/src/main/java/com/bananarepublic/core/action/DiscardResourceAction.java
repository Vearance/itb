package com.bananarepublic.core.action;

import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;

import java.util.Objects;

public final class DiscardResourceAction {
    private final PlayerId playerId;
    private final ResourceBundle discardedResources;

    public DiscardResourceAction(PlayerId playerId, ResourceBundle discardedResources) {
        this.playerId = Objects.requireNonNull(playerId, "playerId");
        this.discardedResources = Objects.requireNonNull(discardedResources, "discardedResources").copy();
    }

    public PlayerId getPlayerId() {
        return playerId;
    }

    public ResourceBundle getDiscardedResources() {
        return discardedResources.copy();
    }
}
