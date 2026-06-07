package com.bananarepublic.core.action;

import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;

import java.util.Objects;

public final class TradeAction {
    private final PlayerId initiatorId;
    private final PlayerId targetId;
    private final ResourceBundle offeredResources;
    private final ResourceBundle requestedResources;

    public TradeAction(PlayerId initiatorId, PlayerId targetId, ResourceBundle offeredResources, ResourceBundle requestedResources) {
        this.initiatorId = Objects.requireNonNull(initiatorId, "initiatorId");
        this.targetId = Objects.requireNonNull(targetId, "targetId");
        this.offeredResources = Objects.requireNonNull(offeredResources, "offeredResources").copy();
        this.requestedResources = Objects.requireNonNull(requestedResources, "requestedResources").copy();
    }

    public PlayerId getInitiatorId() {
        return initiatorId;
    }

    public PlayerId getTargetId() {
        return targetId;
    }

    public ResourceBundle getOfferedResources() {
        return offeredResources.copy();
    }

    public ResourceBundle getRequestedResources() {
        return requestedResources.copy();
    }

    @Override
    public boolean equals(Object other) {
        if (this == other) {
            return true;
        }
        if (!(other instanceof TradeAction action)) {
            return false;
        }
        return initiatorId.equals(action.initiatorId)
                && targetId.equals(action.targetId)
                && offeredResources.equals(action.offeredResources)
                && requestedResources.equals(action.requestedResources);
    }

    @Override
    public int hashCode() {
        return Objects.hash(initiatorId, targetId, offeredResources, requestedResources);
    }
}
