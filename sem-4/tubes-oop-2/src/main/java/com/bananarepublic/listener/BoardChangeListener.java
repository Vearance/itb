package com.bananarepublic.listener;

import com.bananarepublic.event.BuildingPlacedEvent;
import com.bananarepublic.event.BuildingUpgradedEvent;
import com.bananarepublic.event.GameEvent;
import com.bananarepublic.event.GameEventListener;
import com.bananarepublic.event.RobberMovedEvent;

public class BoardChangeListener implements GameEventListener<GameEvent> {
    @Override
    public void onEvent(GameEvent event) {
        if (event instanceof BuildingPlacedEvent buildingPlacedEvent) {
            onBuildingPlaced(buildingPlacedEvent);
        } else if (event instanceof BuildingUpgradedEvent buildingUpgradedEvent) {
            onBuildingUpgraded(buildingUpgradedEvent);
        } else if (event instanceof RobberMovedEvent robberMovedEvent) {
            onRobberMoved(robberMovedEvent);
        }
    }

    protected void onBuildingPlaced(BuildingPlacedEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }

    protected void onBuildingUpgraded(BuildingUpgradedEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }

    protected void onRobberMoved(RobberMovedEvent event) {
        // Default no-op. UI/viewmodel listeners can override this.
    }
}
