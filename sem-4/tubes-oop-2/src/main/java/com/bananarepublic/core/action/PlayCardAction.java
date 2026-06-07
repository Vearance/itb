package com.bananarepublic.core.action;

import com.bananarepublic.core.board.PathId;
import com.bananarepublic.core.card.ExperimentCardId;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceType;

import java.util.List;
import java.util.Objects;

public final class PlayCardAction {
    private final PlayerId playerId;
    private final ExperimentCardId cardId;
    private final String robberHexTileId;
    private final PlayerId targetPlayerId;
    private final List<PathId> freePipePathIds;
    private final ResourceType monopolyResourceType;

    private PlayCardAction(
            PlayerId playerId,
            ExperimentCardId cardId,
            String robberHexTileId,
            PlayerId targetPlayerId,
            List<PathId> freePipePathIds,
            ResourceType monopolyResourceType
    ) {
        this.playerId = Objects.requireNonNull(playerId, "playerId");
        this.cardId = Objects.requireNonNull(cardId, "cardId");
        this.robberHexTileId = robberHexTileId;
        this.targetPlayerId = targetPlayerId;
        this.freePipePathIds = List.copyOf(freePipePathIds == null ? List.of() : freePipePathIds);
        this.monopolyResourceType = monopolyResourceType;
    }

    public static PlayCardAction knight(PlayerId playerId, ExperimentCardId cardId, String robberHexTileId, PlayerId targetPlayerId) {
        return new PlayCardAction(playerId, cardId, robberHexTileId, targetPlayerId, List.of(), null);
    }

    public static PlayCardAction roadBuilding(PlayerId playerId, ExperimentCardId cardId, List<PathId> freePipePathIds) {
        return new PlayCardAction(playerId, cardId, null, null, freePipePathIds, null);
    }

    public static PlayCardAction monopoly(PlayerId playerId, ExperimentCardId cardId, ResourceType monopolyResourceType) {
        return new PlayCardAction(playerId, cardId, null, null, List.of(), monopolyResourceType);
    }

    public static PlayCardAction plugin(PlayerId playerId, ExperimentCardId cardId) {
        return new PlayCardAction(playerId, cardId, null, null, List.of(), null);
    }

    public PlayerId getPlayerId() {
        return playerId;
    }

    public ExperimentCardId getCardId() {
        return cardId;
    }

    public String getRobberHexTileId() {
        return robberHexTileId;
    }

    public PlayerId getTargetPlayerId() {
        return targetPlayerId;
    }

    public List<PathId> getFreePipePathIds() {
        return freePipePathIds;
    }

    public ResourceType getMonopolyResourceType() {
        return monopolyResourceType;
    }
}
