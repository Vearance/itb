package com.bananarepublic.listener;

import com.bananarepublic.event.CardBoughtEvent;
import com.bananarepublic.event.CardPlayedEvent;
import com.bananarepublic.event.GameEvent;
import com.bananarepublic.event.GameEventListener;

public class CardChangeListener implements GameEventListener<GameEvent> {
    @Override
    public void onEvent(GameEvent event) {
        if (event instanceof CardBoughtEvent cardBoughtEvent) {
            onCardBought(cardBoughtEvent);
        } else if (event instanceof CardPlayedEvent cardPlayedEvent) {
            onCardPlayed(cardPlayedEvent);
        }
    }

    protected void onCardBought(CardBoughtEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }

    protected void onCardPlayed(CardPlayedEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }
}
