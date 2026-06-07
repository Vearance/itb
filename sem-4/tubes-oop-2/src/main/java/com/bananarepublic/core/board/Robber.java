package com.bananarepublic.core.board;

import java.util.Objects;

public final class Robber {
    private String hexTileId;

    public Robber(String hexTileId) {
        this.hexTileId = Objects.requireNonNull(hexTileId, "hexTileId");
    }

    public String getHexTileId() {
        return hexTileId;
    }

    public void moveTo(String hexTileId) {
        if (this.hexTileId.equals(hexTileId)) {
            throw new IllegalArgumentException("Nimon Ungu must move to a different tile");
        }
        this.hexTileId = Objects.requireNonNull(hexTileId, "hexTileId");
    }
}
