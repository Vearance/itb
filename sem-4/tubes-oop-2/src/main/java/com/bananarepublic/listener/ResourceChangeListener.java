package com.bananarepublic.listener;

import com.bananarepublic.event.GameEvent;
import com.bananarepublic.event.GameEventListener;
import com.bananarepublic.event.ResourceChangedEvent;
import com.bananarepublic.event.ResourceDiscardedEvent;
import com.bananarepublic.event.ResourceStolenEvent;

public class ResourceChangeListener implements GameEventListener<GameEvent> {
    @Override
    public void onEvent(GameEvent event) {
        if (event instanceof ResourceChangedEvent resourceChangedEvent) {
            onResourceChanged(resourceChangedEvent);
        } else if (event instanceof ResourceDiscardedEvent resourceDiscardedEvent) {
            onResourceDiscarded(resourceDiscardedEvent);
        } else if (event instanceof ResourceStolenEvent resourceStolenEvent) {
            onResourceStolen(resourceStolenEvent);
        }
    }

    protected void onResourceChanged(ResourceChangedEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }

    protected void onResourceDiscarded(ResourceDiscardedEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }

    protected void onResourceStolen(ResourceStolenEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }
}
