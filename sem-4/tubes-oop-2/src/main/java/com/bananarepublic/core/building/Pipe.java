package com.bananarepublic.core.building;

import com.bananarepublic.core.player.PlayerId;

public final class Pipe extends AbstractBuilding {
    public Pipe(PlayerId ownerId) {
        super(ownerId, BuildType.PIPE, 0);
    }
}
