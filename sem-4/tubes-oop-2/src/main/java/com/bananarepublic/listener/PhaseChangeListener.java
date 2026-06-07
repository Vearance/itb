package com.bananarepublic.listener;

import com.bananarepublic.event.GameEventListener;
import com.bananarepublic.event.PhaseChangedEvent;

public class PhaseChangeListener implements GameEventListener<PhaseChangedEvent> {
    @Override
    public void onEvent(PhaseChangedEvent event) {
        onPhaseChanged(event);
    }

    protected void onPhaseChanged(PhaseChangedEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }
}
