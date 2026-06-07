package com.bananarepublic.core.action;

import com.bananarepublic.core.player.PlayerId;

import java.util.Objects;
import java.util.Optional;

public final class MoveRobberAction {
    private final PlayerId playerId;
    private final String targetHexTileId;
    private final PlayerId targetPlayerId;

    public MoveRobberAction(PlayerId playerId, String targetHexTileId, PlayerId targetPlayerId) {
        if (targetHexTileId == null || targetHexTileId.isBlank()) {
            throw new IllegalArgumentException("Target hex tile id cannot be blank");
        }
        this.playerId = Objects.requireNonNull(playerId, "playerId");
        this.targetHexTileId = targetHexTileId;
        this.targetPlayerId = targetPlayerId;
    }

    public PlayerId getPlayerId() {
        return playerId;
    }

    public String getTargetHexTileId() {
        return targetHexTileId;
    }

    public Optional<PlayerId> getTargetPlayerId() {
        return Optional.ofNullable(targetPlayerId);
    }
}
