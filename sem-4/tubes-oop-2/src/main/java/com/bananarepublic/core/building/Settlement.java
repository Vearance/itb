package com.bananarepublic.core.building;

import com.bananarepublic.core.player.PlayerId;

public final class Settlement extends AbstractBuilding {
    public Settlement(PlayerId ownerId) {
        super(ownerId, BuildType.SETTLEMENT, 1);
    }
}
