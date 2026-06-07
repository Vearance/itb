package com.bananarepublic.core.building;

import com.bananarepublic.core.player.PlayerId;

public final class Laboratory extends AbstractBuilding {
    public Laboratory(PlayerId ownerId) {
        super(ownerId, BuildType.LABORATORY, 2);
    }
}
