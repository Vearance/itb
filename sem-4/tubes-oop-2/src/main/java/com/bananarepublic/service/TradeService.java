package com.bananarepublic.service;

import com.bananarepublic.core.action.TradeAction;
import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.GameEventBus;

import java.util.Objects;

public final class TradeService {
    private final NormalTradeService normalTradeService;
    private final HarborTradeService harborTradeService;

    public TradeService(GameEventBus eventBus) {
        this(new NormalTradeService(eventBus), new HarborTradeService(eventBus));
    }

    public TradeService(NormalTradeService normalTradeService, HarborTradeService harborTradeService) {
        this.normalTradeService = Objects.requireNonNull(normalTradeService, "normalTradeService");
        this.harborTradeService = Objects.requireNonNull(harborTradeService, "harborTradeService");
    }

    public TradeAction requestNormalTrade(GameState state, TradeAction action) {
        return normalTradeService.requestTrade(state, action);
    }

    public void acceptNormalTrade(GameState state, TradeAction action) {
        normalTradeService.acceptTrade(state, action);
    }

    public void acceptNormalTrade(GameState state, TradeAction action, PlayerId acceptingPlayerId) {
        normalTradeService.acceptTrade(state, action, acceptingPlayerId);
    }

    public void rejectNormalTrade(TradeAction action, String reason) {
        normalTradeService.rejectTrade(action, reason);
    }

    public void rejectNormalTrade(TradeAction action, PlayerId rejectingPlayerId, String reason) {
        normalTradeService.rejectTrade(action, rejectingPlayerId, reason);
    }

    public TradeAction counterOfferNormalTrade(GameState state, TradeAction originalAction, ResourceBundle newOffered, ResourceBundle newRequested) {
        return normalTradeService.counterOffer(state, originalAction, newOffered, newRequested);
    }

    public TradeAction getPendingNormalTrade() {
        return normalTradeService.getPendingTrade();
    }

    public void cancelPendingNormalTrade(String reason) {
        normalTradeService.cancelPendingTrade(reason);
    }

    public int getBestHarborRatio(Board board, PlayerId playerId, ResourceType offeredType) {
        return harborTradeService.getBestTradeRatio(board, playerId, offeredType);
    }

    public void tradeWithHarbor(GameState state, Board board, ResourceType offeredType, ResourceType requestedType) {
        harborTradeService.tradeWithHarbor(state, board, offeredType, requestedType);
    }

    public int getBestHarborTradeRatio(Board board, PlayerId playerId, ResourceType offeredType) {
        return harborTradeService.getBestTradeRatio(board, playerId, offeredType);
    }
}
