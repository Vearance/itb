package com.bananarepublic.listener;

import com.bananarepublic.event.GameEvent;
import com.bananarepublic.event.GameEventListener;
import com.bananarepublic.event.PlayerTurnEndedEvent;
import com.bananarepublic.event.PlayerTurnStartedEvent;

public class TurnChangeListener implements GameEventListener<GameEvent> {
    @Override
    public void onEvent(GameEvent event) {
        if (event instanceof PlayerTurnStartedEvent turnStartedEvent) {
            onTurnStarted(turnStartedEvent);
        } else if (event instanceof PlayerTurnEndedEvent turnEndedEvent) {
            onTurnEnded(turnEndedEvent);
        }
    }

    protected void onTurnStarted(PlayerTurnStartedEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }

    protected void onTurnEnded(PlayerTurnEndedEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }
}
