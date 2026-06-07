package com.bananarepublic.validator;

import com.bananarepublic.core.action.TradeAction;
import com.bananarepublic.core.game.GamePhase;
import com.bananarepublic.core.game.GameStatus;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.exception.trade.InvalidHarborTradeException;
import com.bananarepublic.exception.trade.InvalidTradeException;

import java.util.Objects;

public final class TradeValidator {
    public void validateNormalTrade(GameState state, TradeAction action) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(action, "action");
        AbstractPlayer initiator = validateNormalTradeExchange(state, action);
        if (!state.getCurrentPlayer().getId().equals(initiator.getId())) {
            throw new InvalidTradeException("Only the active player can start a normal trade");
        }
    }

    public AbstractPlayer validateNormalTradeExchange(GameState state, TradeAction action) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(action, "action");
        requireTradePhase(state);

        AbstractPlayer initiator = state.getPlayerById(action.getInitiatorId());
        AbstractPlayer target = state.getPlayerById(action.getTargetId());
        if (initiator.getId().equals(target.getId())) {
            throw new InvalidTradeException("Player cannot trade with themselves");
        }

        ResourceBundle offeredResources = action.getOfferedResources();
        ResourceBundle requestedResources = action.getRequestedResources();
        validateResourceExchange(offeredResources, requestedResources);

        if (!initiator.getInventory().hasResources(offeredResources)) {
            throw new InvalidTradeException("Active player does not have the offered resources");
        }
        if (!target.getInventory().hasResources(requestedResources)) {
            throw new InvalidTradeException("Target player does not have the requested resources");
        }
        return initiator;
    }

    public void validateHarborTrade(GameState state, ResourceType offeredType, ResourceType requestedType, int ratio) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(offeredType, "offeredType");
        Objects.requireNonNull(requestedType, "requestedType");
        requireTradePhase(state);

        if (ratio < 2 || ratio > 4) {
            throw new InvalidHarborTradeException("Harbor trade ratio must be 2, 3, or 4");
        }
        if (offeredType == requestedType) {
            throw new InvalidTradeException("Cannot trade a resource for the same resource");
        }
        if (state.getCurrentPlayer().getResourceCount(offeredType) < ratio) {
            throw new InvalidTradeException("Active player does not have enough " + offeredType);
        }
        if (!state.getBank().hasResource(requestedType, 1)) {
            throw new InvalidTradeException("Bank does not have requested resource: " + requestedType);
        }
    }

    private void requireTradePhase(GameState state) {
        if (state.getStatus() != GameStatus.IN_PROGRESS) {
            throw new InvalidTradeException("Trade is only allowed while the game is in progress");
        }
        if (state.getPhase() != GamePhase.PLAYER_ACTIONS) {
            throw new InvalidTradeException("Trade is only allowed during player actions");
        }
    }

    private void validateResourceExchange(ResourceBundle offeredResources, ResourceBundle requestedResources) {
        if (offeredResources.total() == 0 || requestedResources.total() == 0) {
            throw new InvalidTradeException("Trade cannot give or request resources for free");
        }
        for (ResourceType type : ResourceType.values()) {
            if (offeredResources.get(type) > 0 && requestedResources.get(type) > 0) {
                throw new InvalidTradeException("Cannot trade the same resource type both ways: " + type);
            }
        }
    }
}
