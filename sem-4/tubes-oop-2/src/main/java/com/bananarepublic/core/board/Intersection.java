package com.bananarepublic.core.board;

import com.bananarepublic.core.building.AbstractBuilding;
import com.bananarepublic.core.building.BuildType;

import java.util.LinkedHashSet;
import java.util.Optional;
import java.util.Set;

public final class Intersection {
    private final IntersectionId id;
    private final BoardPosition position;
    private final Set<String> adjacentHexIds = new LinkedHashSet<>();
    private AbstractBuilding building;

    public Intersection(IntersectionId id, BoardPosition position) {
        this.id = id;
        this.position = position;
    }

    public IntersectionId getId() {
        return id;
    }

    public BoardPosition getPosition() {
        return position;
    }

    public Set<String> getAdjacentHexIds() {
        return Set.copyOf(adjacentHexIds);
    }

    public void addAdjacentHex(String hexId) {
        adjacentHexIds.add(hexId);
    }

    public boolean hasBuilding() {
        return building != null;
    }

    public Optional<AbstractBuilding> getBuilding() {
        return Optional.ofNullable(building);
    }

    public void placeBuilding(AbstractBuilding building) {
        if (hasBuilding()) {
            throw new IllegalStateException("Intersection already has a building");
        }
        if (building.getBuildType() == BuildType.PIPE) {
            throw new IllegalArgumentException("Pipe cannot be placed on an intersection");
        }
        this.building = building;
    }

    public void upgradeBuilding(AbstractBuilding building) {
        if (!hasBuilding()) {
            throw new IllegalStateException("Intersection has no building to upgrade");
        }
        if (building.getBuildType() == BuildType.PIPE) {
            throw new IllegalArgumentException("Pipe cannot be placed on an intersection");
        }
        this.building = building;
    }
}
