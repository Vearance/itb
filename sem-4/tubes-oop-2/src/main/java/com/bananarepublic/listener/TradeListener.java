package com.bananarepublic.listener;

import com.bananarepublic.event.GameEvent;
import com.bananarepublic.event.GameEventListener;
import com.bananarepublic.event.TradeCompletedEvent;
import com.bananarepublic.event.TradeRejectedEvent;
import com.bananarepublic.event.TradeRequestedEvent;

public class TradeListener implements GameEventListener<GameEvent> {
    @Override
    public void onEvent(GameEvent event) {
        if (event instanceof TradeRequestedEvent tradeRequestedEvent) {
            onTradeRequested(tradeRequestedEvent);
        } else if (event instanceof TradeCompletedEvent tradeCompletedEvent) {
            onTradeCompleted(tradeCompletedEvent);
        } else if (event instanceof TradeRejectedEvent tradeRejectedEvent) {
            onTradeRejected(tradeRejectedEvent);
        }
    }

    protected void onTradeRequested(TradeRequestedEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }

    protected void onTradeCompleted(TradeCompletedEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }

    protected void onTradeRejected(TradeRejectedEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }
}
