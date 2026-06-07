package com.bananarepublic.core.board.generator;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.board.TerrainType;

public final class BoardValidator {
    public List<String> validate(Board board) {
        List<String> errors = new ArrayList<>();
        if (board.getHexTiles().size() != 19) {
            errors.add("Board must contain exactly 19 hex tiles");
        }

        long gurunCount = board.getHexTiles().values().stream()
                .filter(hex -> hex.getTerrainType() == TerrainType.GURUN)
                .count();
        if (gurunCount != 1) {
            errors.add("Board must contain exactly 1 Gurun");
        }

        board.getHexTiles().values().forEach(hex -> hex.getTokenNumber().ifPresent(token -> {
            if (token.getValue() == 7) {
                errors.add("Token number 7 is not allowed");
            }
        }));

        Map<Integer, Long> tokenCounts = board.getHexTiles().values().stream()
                .flatMap(hex -> hex.getTokenNumber().stream())
                .collect(Collectors.groupingBy(token -> token.getValue(), Collectors.counting()));
        Map<Integer, Long> expected = Map.of(
                2, 1L, 3, 2L, 4, 2L, 5, 2L, 6, 2L,
                8, 2L, 9, 2L, 10, 2L, 11, 2L, 12, 1L
        );
        if (!tokenCounts.equals(expected)) {
            errors.add("Token number distribution is invalid");
        }

        board.getHexTiles().values().forEach(hex -> {
            if (hex.getIntersectionIds().size() != 6) {
                errors.add("Hex " + hex.getId() + " must have 6 intersections");
            }
        });

        board.getIntersections().values().forEach(intersection -> {
            if (intersection.hasBuilding()) {
                errors.add("Intersection " + intersection.getId().value() + " must start without a building");
            }
        });

        board.getPaths().values().forEach(path -> {
            if (!board.getIntersections().containsKey(path.getFirstIntersectionId())
                    || !board.getIntersections().containsKey(path.getSecondIntersectionId())) {
                errors.add("Path " + path.getId().value() + " references missing intersection");
            }
            if (path.hasPipe()) {
                errors.add("Path " + path.getId().value() + " must start without a pipe");
            }
        });

        board.getHarbors().forEach(harbor -> harbor.getIntersectionIds().forEach(intersectionId -> {
            if (!board.getIntersections().containsKey(intersectionId)) {
                errors.add("Harbor " + harbor.getId() + " references missing intersection");
            }
        }));
        String robberHexId = board.getRobber().getHexTileId();
        if (board.getHexTile(robberHexId).isEmpty()
                || board.getHexTile(robberHexId).orElseThrow().getTerrainType() != TerrainType.GURUN) {
            errors.add("Nimon Ungu must start on Gurun");
        }
        return errors;
    }
}
