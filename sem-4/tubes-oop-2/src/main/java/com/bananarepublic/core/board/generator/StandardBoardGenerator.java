package com.bananarepublic.core.board.generator;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.board.BoardPosition;
import com.bananarepublic.core.board.Harbor;
import com.bananarepublic.core.board.HarborType;
import com.bananarepublic.core.board.HexCoordinate;
import com.bananarepublic.core.board.HexTile;
import com.bananarepublic.core.board.Intersection;
import com.bananarepublic.core.board.IntersectionId;
import com.bananarepublic.core.board.Path;
import com.bananarepublic.core.board.PathId;
import com.bananarepublic.core.board.Robber;
import com.bananarepublic.core.board.TerrainType;
import com.bananarepublic.core.board.TokenNumber;

public final class StandardBoardGenerator implements BoardGenerator {
    private static final double HEX_SIZE = 100.0;
    private static final TerrainType[] TERRAINS = {
            TerrainType.HUTAN, TerrainType.BUKIT, TerrainType.LADANG, TerrainType.GUNUNG, TerrainType.KEBUN_PISANG,
            TerrainType.HUTAN, TerrainType.LADANG, TerrainType.BUKIT, TerrainType.KEBUN_PISANG, TerrainType.GUNUNG,
            TerrainType.GURUN,
            TerrainType.HUTAN, TerrainType.LADANG, TerrainType.KEBUN_PISANG, TerrainType.BUKIT, TerrainType.GUNUNG,
            TerrainType.HUTAN, TerrainType.LADANG, TerrainType.KEBUN_PISANG
    };
    private static final int[] TOKENS = {5, 2, 6, 3, 8, 10, 9, 12, 11, 4, 0, 8, 10, 9, 4, 5, 6, 3, 11};

    @Override
    public BoardGenerationResult generate() {
        Map<String, HexTile> hexes = new LinkedHashMap<>();
        Map<IntersectionId, Intersection> intersections = new LinkedHashMap<>();
        Map<PathId, Path> paths = new LinkedHashMap<>();
        List<HexCoordinate> coordinates = standardCoordinates();

        String gurunHexId = null;
        for (int i = 0; i < coordinates.size(); i++) {
            HexCoordinate coordinate = coordinates.get(i);
            TerrainType terrain = TERRAINS[i];
            String hexId = "H" + (i + 1);
            List<IntersectionId> cornerIds = new ArrayList<>();

            // Create intersections for each corner of the hex
            for (int corner = 0; corner < 6; corner++) {
                BoardPosition position = cornerPosition(coordinate, corner);
                IntersectionId intersectionId = intersectionId(position);
                intersections.computeIfAbsent(intersectionId, id -> new Intersection(id, position)).addAdjacentHex(hexId);
                cornerIds.add(intersectionId);
            }

            // Create paths between adjacent corners
            for (int corner = 0; corner < 6; corner++) {
                IntersectionId first = cornerIds.get(corner);
                IntersectionId second = cornerIds.get((corner + 1) % 6);
                PathId pathId = pathId(first, second);
                paths.putIfAbsent(pathId, new Path(pathId, first, second));
            }

            // Create hex tile with token number
            TokenNumber token = terrain == TerrainType.GURUN ? null : new TokenNumber(TOKENS[i]);
            if (terrain == TerrainType.GURUN) {
                gurunHexId = hexId;
            }
            hexes.put(hexId, new HexTile(hexId, coordinate, terrain, token, cornerIds));
        }

        Board board = new Board(hexes, intersections, paths, createHarbors(intersections, paths), new Robber(gurunHexId));
        return new BoardGenerationResult(board, new BoardValidator().validate(board));
    }

    private List<HexCoordinate> standardCoordinates() {
        List<HexCoordinate> coordinates = new ArrayList<>();
        for (int q = -2; q <= 2; q++) {
            for (int r = -2; r <= 2; r++) {
                if (Math.abs(q + r) <= 2) {
                    coordinates.add(new HexCoordinate(q, r));
                }
            }
        }
        coordinates.sort(Comparator.comparingInt(HexCoordinate::r).thenComparingInt(HexCoordinate::q));
        return coordinates;
    }

    private BoardPosition cornerPosition(HexCoordinate coordinate, int corner) {
        double centerX = HEX_SIZE * Math.sqrt(3) * (coordinate.q() + coordinate.r() / 2.0);
        double centerY = HEX_SIZE * 1.5 * coordinate.r();
        double angle = Math.toRadians(60 * corner - 30);
        return new BoardPosition(centerX + HEX_SIZE * Math.cos(angle), centerY + HEX_SIZE * Math.sin(angle));
    }

    private IntersectionId intersectionId(BoardPosition position) {
        return new IntersectionId(Math.round(position.x() * 1000) + ":" + Math.round(position.y() * 1000));
    }

    private PathId pathId(IntersectionId first, IntersectionId second) {
        String a = first.value();
        String b = second.value();
        return a.compareTo(b) <= 0 ? new PathId(a + "|" + b) : new PathId(b + "|" + a);
    }

    private List<Harbor> createHarbors(Map<IntersectionId, Intersection> intersections, Map<PathId, Path> paths) {
        /*
         * A harbor belongs to one coastal edge, not to two arbitrary coastal vertices.
         * A coastal edge is a path whose two endpoint intersections touch exactly one
         * common hex tile. Those two endpoints are the at-most two settlement/lab spots
         * that can access the harbor ability.
         */
        List<CoastalEdge> coastalEdges = paths.values().stream()
                .map(path -> toCoastalEdge(path, intersections))
                .flatMap(Optional::stream)
                .sorted(Comparator.comparingDouble(CoastalEdge::angle))
                .toList();

        HarborType[] types = {
                HarborType.UMUM_3_1, HarborType.KAYU_2_TO_1, HarborType.UMUM_3_1,
                HarborType.BATU_BATA_2_TO_1, HarborType.GANDUM_2_TO_1, HarborType.UMUM_3_1,
                HarborType.BIJIH_2_TO_1, HarborType.PISANG_2_TO_1, HarborType.UMUM_3_1
        };

        List<Harbor> harbors = new ArrayList<>();
        for (int i = 0; i < types.length && !coastalEdges.isEmpty(); i++) {
            int edgeIndex = (int) Math.floor(i * coastalEdges.size() / (double) types.length);
            CoastalEdge edge = coastalEdges.get(Math.min(edgeIndex, coastalEdges.size() - 1));
            harbors.add(new Harbor("HB" + (i + 1), types[i], List.of(edge.first(), edge.second())));
        }
        return harbors;
    }

    private Optional<CoastalEdge> toCoastalEdge(Path path, Map<IntersectionId, Intersection> intersections) {
        Intersection first = intersections.get(path.getFirstIntersectionId());
        Intersection second = intersections.get(path.getSecondIntersectionId());
        if (first == null || second == null) {
            return Optional.empty();
        }

        List<String> commonAdjacentHexes = first.getAdjacentHexIds().stream()
                .filter(second.getAdjacentHexIds()::contains)
                .toList();
        if (commonAdjacentHexes.size() != 1) {
            return Optional.empty();
        }

        BoardPosition midpoint = new BoardPosition(
                (first.getPosition().x() + second.getPosition().x()) / 2.0,
                (first.getPosition().y() + second.getPosition().y()) / 2.0
        );
        double angle = Math.atan2(midpoint.y(), midpoint.x());
        return Optional.of(new CoastalEdge(path.getFirstIntersectionId(), path.getSecondIntersectionId(), angle));
    }

    private record CoastalEdge(IntersectionId first, IntersectionId second, double angle) { }
}
