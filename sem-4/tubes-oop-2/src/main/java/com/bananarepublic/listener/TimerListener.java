package com.bananarepublic.listener;

import com.bananarepublic.event.GameEvent;
import com.bananarepublic.event.GameEventListener;
import com.bananarepublic.event.TimerExpiredEvent;
import com.bananarepublic.event.TimerStartedEvent;
import com.bananarepublic.event.TimerTickEvent;

public class TimerListener implements GameEventListener<GameEvent> {
    @Override
    public void onEvent(GameEvent event) {
        if (event instanceof TimerStartedEvent timerStartedEvent) {
            onTimerStarted(timerStartedEvent);
        } else if (event instanceof TimerTickEvent timerTickEvent) {
            onTimerTick(timerTickEvent);
        } else if (event instanceof TimerExpiredEvent timerExpiredEvent) {
            onTimerExpired(timerExpiredEvent);
        }
    }

    protected void onTimerStarted(TimerStartedEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }

    protected void onTimerTick(TimerTickEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }

    protected void onTimerExpired(TimerExpiredEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }
}
