package com.bananarepublic.service;

import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.board.Harbor;
import com.bananarepublic.core.board.Intersection;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.ResourceChangedEvent;
import com.bananarepublic.event.TradeCompletedEvent;
import com.bananarepublic.validator.TradeValidator;

import java.util.Objects;

public final class HarborTradeService {
    private static final int DEFAULT_RATIO = 4;

    private final GameEventBus eventBus;
    private final TradeValidator tradeValidator;

    public HarborTradeService(GameEventBus eventBus) {
        this(eventBus, new TradeValidator());
    }

    public HarborTradeService(GameEventBus eventBus, TradeValidator tradeValidator) {
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
        this.tradeValidator = Objects.requireNonNull(tradeValidator, "tradeValidator");
    }

    public int getBestTradeRatio(Board board, PlayerId playerId, ResourceType offeredType) {
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(playerId, "playerId");
        Objects.requireNonNull(offeredType, "offeredType");

        int bestRatio = DEFAULT_RATIO;
        for (Harbor harbor : board.getHarbors()) {
            if (!playerOwnsHarbor(board, harbor, playerId)) {
                continue;
            }

            if (harbor.getType().getResourceType().isEmpty()) {
                bestRatio = Math.min(bestRatio, harbor.getType().getRatio());
            } else if (harbor.getType().getResourceType().orElseThrow() == offeredType) {
                bestRatio = Math.min(bestRatio, harbor.getType().getRatio());
            }
        }
        return bestRatio;
    }

    public void tradeWithHarbor(GameState state, Board board, ResourceType offeredType, ResourceType requestedType) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        int ratio = getBestTradeRatio(board, state.getCurrentPlayer().getId(), offeredType);
        tradeValidator.validateHarborTrade(state, offeredType, requestedType, ratio);

        AbstractPlayer player = state.getCurrentPlayer();
        ResourceBundle offeredResources = ResourceBundle.of(offeredType, ratio);
        ResourceBundle receivedResources = ResourceBundle.of(requestedType, 1);

        player.getInventory().spendResourceBundle(offeredResources);
        state.getBank().receiveResourceBundle(offeredResources);
        state.getBank().giveResourceBundle(receivedResources);
        player.getInventory().addResourceBundle(receivedResources);

        publishResourceChanges(player, offeredResources, -1);
        publishResourceChanges(player, receivedResources, 1);
        eventBus.publish(new TradeCompletedEvent(
                player.getId(),
                null,
                offeredResources,
                receivedResources,
                true
        ));
    }

    private boolean playerOwnsHarbor(Board board, Harbor harbor, PlayerId playerId) {
        for (var intersectionId : harbor.getIntersectionIds()) {
            Intersection intersection = board.getIntersections().get(intersectionId);
            if (intersection == null) {
                continue;
            }
            if (intersection.getBuilding()
                    .map(building -> building.getOwnerId().equals(playerId))
                    .orElse(false)) {
                return true;
            }
        }
        return false;
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
