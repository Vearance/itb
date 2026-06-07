package com.bananarepublic.ui.controller;

import com.bananarepublic.core.board.PathId;
import com.bananarepublic.core.card.ExperimentCardId;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public record PendingRoadBuildingCardSelection(ExperimentCardId cardId, List<PathId> placedPathIds) {
    public PendingRoadBuildingCardSelection(ExperimentCardId cardId) {
        this(cardId, List.of());
    }

    public PendingRoadBuildingCardSelection {
        Objects.requireNonNull(cardId, "cardId");
        placedPathIds = List.copyOf(Objects.requireNonNull(placedPathIds, "placedPathIds"));
    }

    public int placedCount() {
        return placedPathIds.size();
    }

    public boolean contains(PathId pathId) {
        return placedPathIds.contains(pathId);
    }

    public PendingRoadBuildingCardSelection withPlacedPath(PathId pathId) {
        Objects.requireNonNull(pathId, "pathId");
        List<PathId> nextPaths = new ArrayList<>(placedPathIds);
        nextPaths.add(pathId);
        return new PendingRoadBuildingCardSelection(cardId, nextPaths);
    }
}
