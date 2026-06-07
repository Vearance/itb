package com.bananarepublic.core.board;

import java.util.List;
import java.util.Map;
import java.util.Optional;

public final class Board {
    private final Map<String, HexTile> hexTiles;
    private final Map<IntersectionId, Intersection> intersections;
    private final Map<PathId, Path> paths;
    private final List<Harbor> harbors;
    private final Robber robber;

    public Board(Map<String, HexTile> hexTiles, Map<IntersectionId, Intersection> intersections, Map<PathId, Path> paths, List<Harbor> harbors, Robber robber) {
        this.hexTiles = Map.copyOf(hexTiles);
        this.intersections = Map.copyOf(intersections);
        this.paths = Map.copyOf(paths);
        this.harbors = List.copyOf(harbors);
        this.robber = robber;
    }

    public Map<String, HexTile> getHexTiles() {
        return hexTiles;
    }

    public Optional<HexTile> getHexTile(String id) {
        return Optional.ofNullable(hexTiles.get(id));
    }

    public Map<IntersectionId, Intersection> getIntersections() {
        return intersections;
    }

    public Map<PathId, Path> getPaths() {
        return paths;
    }

    public List<Harbor> getHarbors() {
        return harbors;
    }

    public Robber getRobber() {
        return robber;
    }
}
