package com.bananarepublic.data;

import java.util.List;
import java.util.Map;

public final record SaveData(
        String version,
        long timestamp,
        BoardData board,
        List<PlayerData> players,
        int currentPlayerIndex,
        int turnNumber,
        int initialForwardPlacementCount,
        int initialReversePlacementCount,
        boolean experimentCardPlayedThisTurn,
        BankData bank,
        DeckData deck,
        String robberHexTileId,
        String phase,
        String status,
        int turnTimerRemainingSeconds,
        boolean turnTimerRunning,
        String longestRoadOwnerId,
        String largestArmyOwnerId
) {
    public static final String FORMAT_VERSION = "1.0";

    public SaveData {
        players = players == null ? List.of() : List.copyOf(players);
    }

    public record BoardData(
            Map<String, HexTileData> hexTiles,
            Map<String, IntersectionData> intersections,
            Map<String, PathData> paths,
            List<HarborData> harbors
    ) {
        public BoardData {
            hexTiles = hexTiles == null ? Map.of() : Map.copyOf(hexTiles);
            intersections = intersections == null ? Map.of() : Map.copyOf(intersections);
            paths = paths == null ? Map.of() : Map.copyOf(paths);
            harbors = harbors == null ? List.of() : List.copyOf(harbors);
        }
    }

    public record HexTileData(
            int q,
            int r,
            String terrain,
            Integer tokenValue,
            List<String> intersectionIds
    ) {
        public HexTileData {
            intersectionIds = intersectionIds == null ? List.of() : List.copyOf(intersectionIds);
        }
    }

    public record IntersectionData(
            double x,
            double y,
            List<String> adjacentHexIds,
            BuildingData building
    ) {
        public IntersectionData {
            adjacentHexIds = adjacentHexIds == null ? List.of() : List.copyOf(adjacentHexIds);
        }
    }

    public record BuildingData(
            String ownerId,
            String buildType
    ) {}

    public record PathData(
            String firstIntersectionId,
            String secondIntersectionId,
            BuildingData pipe
    ) {}

    public record HarborData(
            String id,
            String type,
            List<String> intersectionIds
    ) {
        public HarborData {
            intersectionIds = intersectionIds == null ? List.of() : List.copyOf(intersectionIds);
        }
    }

    public record PlayerData(
            String id,
            String name,
            String color,
            Map<String, Integer> resources,
            int settlements,
            int laboratories,
            int pipes,
            int publicPoints,
            int hiddenPoints,
            List<OwnedCardData> cards,
            int playedKnightCount,
            int longestRoadLength
    ) {
        public PlayerData {
            resources = resources == null ? Map.of() : Map.copyOf(resources);
            cards = cards == null ? List.of() : List.copyOf(cards);
        }
    }

    public record OwnedCardData(
            String cardId,
            String cardType,
            int boughtTurnNumber
    ) {}

    public record BankData(
            Map<String, Integer> resources
    ) {
        public BankData {
            resources = resources == null ? Map.of() : Map.copyOf(resources);
        }
    }

    public record DeckData(
            int remainingCount,
            List<String> cardIds
    ) {
        public DeckData {
            cardIds = cardIds == null ? List.of() : List.copyOf(cardIds);
        }
    }
}
