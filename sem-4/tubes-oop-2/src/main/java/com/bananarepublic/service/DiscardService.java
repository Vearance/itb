package com.bananarepublic.service;

import com.bananarepublic.core.action.DiscardResourceAction;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.ResourceChangedEvent;
import com.bananarepublic.event.ResourceDiscardedEvent;
import com.bananarepublic.validator.RobberValidator;

import java.util.List;
import java.util.Objects;

public final class DiscardService {
    private final RobberValidator robberValidator;
    private final GameEventBus eventBus;

    public DiscardService(GameEventBus eventBus) {
        this(new RobberValidator(), eventBus);
    }

    public DiscardService(RobberValidator robberValidator, GameEventBus eventBus) {
        this.robberValidator = Objects.requireNonNull(robberValidator, "robberValidator");
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
    }

    public void discardResources(GameState state, DiscardResourceAction action) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(action, "action");
        robberValidator.validateDiscardAction(state, action);

        AbstractPlayer player = state.getPlayerById(action.getPlayerId());
        ResourceBundle discardedResources = action.getDiscardedResources();
        player.getInventory().spendResourceBundle(discardedResources);
        state.getBank().receiveResourceBundle(discardedResources);
        state.markPlayerDiscarded(player.getId());

        for (ResourceType type : ResourceType.values()) {
            int amount = discardedResources.get(type);
            if (amount == 0) {
                continue;
            }
            eventBus.publish(new ResourceChangedEvent(player.getId(), type, -amount, player.getResourceCount(type)));
        }
        eventBus.publish(new ResourceDiscardedEvent(
                player.getId(),
                discardedResources,
                player.getInventory().getTotalResourceCount()
        ));
    }

    public List<AbstractPlayer> getPlayersRequiredToDiscard(GameState state) {
        return robberValidator.getPlayersRequiredToDiscard(state);
    }

    // Returns true if there are any players that still need to discard resources, false otherwise. example: player A has 8 resources and needs to discard, player B has 3 resources and does not need to discard, this method will return true until player A discards down to 7 or fewer resources.
    public boolean hasPendingDiscards(GameState state) {
        return !getPlayersRequiredToDiscard(state).isEmpty();
    }

    public int getRequiredDiscardCount(AbstractPlayer player) {
        return robberValidator.getRequiredDiscardCount(player);
    }
}
