package com.bananarepublic.service;

import com.bananarepublic.core.action.TradeAction;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.ResourceChangedEvent;
import com.bananarepublic.event.TradeCompletedEvent;
import com.bananarepublic.event.TradeRejectedEvent;
import com.bananarepublic.event.TradeRequestedEvent;
import com.bananarepublic.exception.trade.InvalidTradeException;
import com.bananarepublic.validator.TradeValidator;

import java.util.Objects;
import java.util.function.Consumer;

public final class NormalTradeService {
    private final GameEventBus eventBus;
    private final TradeValidator tradeValidator;
    private TradeAction pendingTrade;

    public NormalTradeService(GameEventBus eventBus) {
        this(eventBus, new TradeValidator());
    }

    public NormalTradeService(GameEventBus eventBus, TradeValidator tradeValidator) {
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
        this.tradeValidator = Objects.requireNonNull(tradeValidator, "tradeValidator");
    }

    public TradeAction requestTrade(GameState state, TradeAction action) {
        tradeValidator.validateNormalTrade(state, action);
        if (pendingTrade != null) {
            throw new InvalidTradeException("There is already a pending trade request");
        }
        pendingTrade = action;
        eventBus.publish(new TradeRequestedEvent(action));
        return action;
    }

    public void acceptTrade(GameState state, TradeAction action) {
        acceptTrade(state, action, action.getTargetId());
    }

    public void acceptTrade(GameState state, TradeAction action, PlayerId acceptingPlayerId) {
        Objects.requireNonNull(acceptingPlayerId, "acceptingPlayerId");
        requirePendingTrade(action);
        if (!acceptingPlayerId.equals(action.getTargetId())) {
            throw new InvalidTradeException("Only the target player can accept this trade");
        }
        tradeValidator.validateNormalTradeExchange(state, action);

        AbstractPlayer initiator = state.getPlayerById(action.getInitiatorId());
        AbstractPlayer target = state.getPlayerById(action.getTargetId());
        ResourceBundle offeredResources = action.getOfferedResources();
        ResourceBundle requestedResources = action.getRequestedResources();

        initiator.getInventory().spendResourceBundle(offeredResources);
        target.getInventory().addResourceBundle(offeredResources);
        target.getInventory().spendResourceBundle(requestedResources);
        initiator.getInventory().addResourceBundle(requestedResources);

        publishResourceChanges(initiator, offeredResources, -1);
        publishResourceChanges(target, offeredResources, 1);
        publishResourceChanges(target, requestedResources, -1);
        publishResourceChanges(initiator, requestedResources, 1);
        eventBus.publish(new TradeCompletedEvent(
                initiator.getId(),
                target.getId(),
                offeredResources,
                requestedResources,
                false
        ));
        pendingTrade = null;
    }

    public void rejectTrade(TradeAction action, String reason) {
        rejectTrade(action, action.getTargetId(), reason);
    }

    public void rejectTrade(TradeAction action, PlayerId rejectingPlayerId, String reason) {
        Objects.requireNonNull(rejectingPlayerId, "rejectingPlayerId");
        requirePendingTrade(action);
        if (!rejectingPlayerId.equals(action.getTargetId())) {
            throw new InvalidTradeException("Only the target player can reject this trade");
        }
        eventBus.publish(new TradeRejectedEvent(action, reason));
        pendingTrade = null;
    }

    public TradeAction counterOffer(GameState state, TradeAction originalAction, ResourceBundle newOffered, ResourceBundle newRequested) {
        requirePendingTrade(originalAction);

        // Validate the counter-offer's resource composition (same rules except who initiates)
        if (newOffered.total() == 0 || newRequested.total() == 0) {
            throw new InvalidTradeException("Counter-offer cannot give or request resources for free");
        }
        for (ResourceType type : ResourceType.values()) {
            if (newOffered.get(type) > 0 && newRequested.get(type) > 0) {
                throw new InvalidTradeException("Cannot trade the same resource type both ways: " + type);
            }
        }
        AbstractPlayer counterInitiator = state.getPlayerById(originalAction.getTargetId());
        if (!counterInitiator.getInventory().hasResources(newOffered)) {
            throw new InvalidTradeException("You do not have the offered resources");
        }
        AbstractPlayer other = state.getPlayerById(originalAction.getInitiatorId());
        if (!other.getInventory().hasResources(newRequested)) {
            throw new InvalidTradeException("Target does not have the requested resources");
        }

        // Cancel current pending trade
        TradeAction oldTrade = pendingTrade;
        pendingTrade = null;
        eventBus.publish(new TradeRejectedEvent(oldTrade, "Counter-offer proposed"));

        // Create counter with swapped roles: original target becomes initiator
        TradeAction counter = new TradeAction(
                originalAction.getTargetId(),
                originalAction.getInitiatorId(),
                newOffered,
                newRequested
        );
        pendingTrade = counter;
        eventBus.publish(new TradeRequestedEvent(counter));
        return counter;
    }

    public TradeAction getPendingTrade() {
        return pendingTrade;
    }

    public void cancelPendingTrade(String reason) {
        if (pendingTrade == null) {
            return;
        }
        TradeAction canceledTrade = pendingTrade;
        pendingTrade = null;
        eventBus.publish(new TradeRejectedEvent(canceledTrade, reason));
    }

    private void requirePendingTrade(TradeAction action) {
        Objects.requireNonNull(action, "action");
        if (pendingTrade == null) {
            throw new InvalidTradeException("There is no pending trade request");
        }
        if (!pendingTrade.equals(action)) {
            throw new InvalidTradeException("Trade action does not match the pending request");
        }
    }

    private void publishResourceChanges(AbstractPlayer player, ResourceBundle resources, int direction) {
        for (ResourceType type : ResourceType.values()) {
            int amount = resources.get(type);
            if (amount == 0) {
                continue;
            }
            eventBus.publish(new ResourceChangedEvent(
                    player.getId(),
                    type,
                    amount * direction,
                    player.getResourceCount(type)
            ));
        }
    }
}
