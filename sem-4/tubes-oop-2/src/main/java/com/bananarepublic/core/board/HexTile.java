package com.bananarepublic.core.board;

import java.util.List;
import java.util.Objects;
import java.util.Optional;

public final class HexTile {
    private final String id;
    private final HexCoordinate coordinate;
    private final TerrainType terrainType;
    private final TokenNumber tokenNumber;
    private final List<IntersectionId> intersectionIds;

    public HexTile(String id, HexCoordinate coordinate, TerrainType terrainType, TokenNumber tokenNumber, List<IntersectionId> intersectionIds) {
        if (terrainType == TerrainType.GURUN && tokenNumber != null) {
            throw new IllegalArgumentException("Gurun tile cannot have token number");
        }
        if (terrainType != TerrainType.GURUN && tokenNumber == null) {
            throw new IllegalArgumentException("Resource tile must have token number");
        }
        this.id = Objects.requireNonNull(id, "id");
        this.coordinate = Objects.requireNonNull(coordinate, "coordinate");
        this.terrainType = Objects.requireNonNull(terrainType, "terrainType");
        this.tokenNumber = tokenNumber;
        this.intersectionIds = List.copyOf(intersectionIds);
    }

    public String getId() {
        return id;
    }

    public HexCoordinate getCoordinate() {
        return coordinate;
    }

    public TerrainType getTerrainType() {
        return terrainType;
    }

    public Optional<TokenNumber> getTokenNumber() {
        return Optional.ofNullable(tokenNumber);
    }

    public List<IntersectionId> getIntersectionIds() {
        return intersectionIds;
    }
}
