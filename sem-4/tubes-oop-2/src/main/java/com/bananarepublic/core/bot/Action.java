package com.bananarepublic.core.bot;

import com.bananarepublic.core.resource.ResourceType;

import java.util.Objects;

public final class Action {
    private final ActionType type;
    private final ResourceType offeredResourceType;
    private final ResourceType requestedResourceType;

    private Action(ActionType type, ResourceType offeredResourceType, ResourceType requestedResourceType) {
        this.type = Objects.requireNonNull(type, "type");
        this.offeredResourceType = offeredResourceType;
        this.requestedResourceType = requestedResourceType;
        validate();
    }

    public static Action noOp() {
        return new Action(ActionType.NO_OP, null, null);
    }

    public static Action endTurn() {
        return new Action(ActionType.END_TURN, null, null);
    }

    public static Action buyExperimentCard() {
        return new Action(ActionType.BUY_EXPERIMENT_CARD, null, null);
    }

    public static Action harborTrade(ResourceType offeredResourceType, ResourceType requestedResourceType) {
        return new Action(ActionType.HARBOR_TRADE, offeredResourceType, requestedResourceType);
    }

    public ActionType getType() {
        return type;
    }

    public ResourceType getOfferedResourceType() {
        return offeredResourceType;
    }

    public ResourceType getRequestedResourceType() {
        return requestedResourceType;
    }

    private void validate() {
        if (type != ActionType.HARBOR_TRADE) {
            return;
        }
        Objects.requireNonNull(offeredResourceType, "offeredResourceType");
        Objects.requireNonNull(requestedResourceType, "requestedResourceType");
        if (offeredResourceType == requestedResourceType) {
            throw new IllegalArgumentException("Harbor trade must exchange different resource types");
        }
    }
}
