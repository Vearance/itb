package com.bananarepublic.service;

import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.board.BoardPosition;
import com.bananarepublic.core.board.Harbor;
import com.bananarepublic.core.board.HarborType;
import com.bananarepublic.core.board.HexCoordinate;
import com.bananarepublic.core.board.HexTile;
import com.bananarepublic.core.board.Intersection;
import com.bananarepublic.core.board.IntersectionId;
import com.bananarepublic.core.board.PathId;
import com.bananarepublic.core.board.Robber;
import com.bananarepublic.core.board.TerrainType;
import com.bananarepublic.core.board.TokenNumber;
import com.bananarepublic.core.building.AbstractBuilding;
import com.bananarepublic.core.building.BuildType;
import com.bananarepublic.core.building.BuildingFactory;
import com.bananarepublic.core.card.CardFactory;
import com.bananarepublic.core.card.CardType;
import com.bananarepublic.core.card.DevelopmentDeck;
import com.bananarepublic.core.card.ExperimentCard;
import com.bananarepublic.core.card.ExperimentCardId;
import com.bananarepublic.core.card.OwnedExperimentCard;
import com.bananarepublic.core.game.GamePhase;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.game.GameStatus;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerColor;
import com.bananarepublic.core.player.PlayerFactory;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBank;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.data.SaveData;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;

public final class PersistenceService {
    private static final String SAVE_FILE_SUFFIX = ".banana-republic.json";

    private final Gson gson;

    public PersistenceService() {
        this.gson = new GsonBuilder()
                .setPrettyPrinting()
                .disableHtmlEscaping()
                .create();
    }

    public void save(GameState state, Board board, Path filePath) throws IOException {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(filePath, "filePath");

        SaveData data = toSaveData(state, board);
        String json = gson.toJson(data);

        Path resolved = ensureSuffix(filePath);
        Files.createDirectories(resolved.getParent());
        Files.writeString(resolved, json, StandardCharsets.UTF_8);
    }

    public Path save(GameState state, Board board, Path directory, String label) throws IOException {
        Objects.requireNonNull(directory, "directory");
        String fileName = sanitizeFileName(label) + SAVE_FILE_SUFFIX;
        Path filePath = directory.resolve(fileName);
        save(state, board, filePath);
        return filePath;
    }

    public SavedGame load(Path filePath) throws IOException {
        Objects.requireNonNull(filePath, "filePath");

        String json = Files.readString(filePath, StandardCharsets.UTF_8);
        SaveData data = gson.fromJson(json, SaveData.class);
        if (data == null) {
            throw new IOException("Save file is empty or malformed");
        }
        return fromSaveData(data);
    }

    private SaveData toSaveData(GameState state, Board board) {
        return new SaveData(
                SaveData.FORMAT_VERSION,
                System.currentTimeMillis(),
                toBoardData(board),
                state.getPlayers().stream().map(this::toPlayerData).toList(),
                state.getCurrentPlayerIndex(),
                state.getTurnNumber(),
                state.getInitialForwardPlacementCount(),
                state.getInitialReversePlacementCount(),
                state.hasPlayedExperimentCardThisTurn(),
                toBankData(state),
                toDeckData(state),
                board.getRobber().getHexTileId(),
                state.getPhase().name(),
                state.getStatus().name(),
                state.getTurnTimerRemainingSeconds(),
                state.isTurnTimerRunning(),
                state.getLongestRoadOwnerId() != null ? state.getLongestRoadOwnerId().getValue() : null,
                state.getLargestArmyOwnerId() != null ? state.getLargestArmyOwnerId().getValue() : null
        );
    }

    private SaveData.BoardData toBoardData(Board board) {
        Map<String, SaveData.HexTileData> hexTiles = new LinkedHashMap<>();
        for (HexTile hex : board.getHexTiles().values()) {
            hexTiles.put(hex.getId(), new SaveData.HexTileData(
                    hex.getCoordinate().q(),
                    hex.getCoordinate().r(),
                    hex.getTerrainType().name(),
                    hex.getTokenNumber().map(TokenNumber::getValue).orElse(null),
                    hex.getIntersectionIds().stream().map(IntersectionId::value).toList()
            ));
        }

        Map<String, SaveData.IntersectionData> intersections = new LinkedHashMap<>();
        for (Intersection intersection : board.getIntersections().values()) {
            intersections.put(intersection.getId().value(), new SaveData.IntersectionData(
                    intersection.getPosition().x(),
                    intersection.getPosition().y(),
                    List.copyOf(intersection.getAdjacentHexIds()),
                    intersection.getBuilding().map(this::toBuildingData).orElse(null)
            ));
        }

        Map<String, SaveData.PathData> paths = new LinkedHashMap<>();
        for (com.bananarepublic.core.board.Path bp : board.getPaths().values()) {
            paths.put(bp.getId().value(), new SaveData.PathData(
                    bp.getFirstIntersectionId().value(),
                    bp.getSecondIntersectionId().value(),
                    bp.getPipe().map(pipe -> toBuildingData(pipe)).orElse(null)
            ));
        }

        List<SaveData.HarborData> harbors = board.getHarbors().stream()
                .map(h -> new SaveData.HarborData(
                        h.getId(),
                        h.getType().name(),
                        h.getIntersectionIds().stream().map(IntersectionId::value).toList()
                ))
                .toList();

        return new SaveData.BoardData(hexTiles, intersections, paths, harbors);
    }

    private SaveData.BuildingData toBuildingData(AbstractBuilding building) {
        return new SaveData.BuildingData(
                building.getOwnerId().getValue(),
                building.getBuildType().name()
        );
    }

    private SaveData.PlayerData toPlayerData(AbstractPlayer player) {
        Map<String, Integer> resources = new HashMap<>();
        for (ResourceType type : ResourceType.values()) {
            int count = player.getResourceCount(type);
            if (count > 0) {
                resources.put(type.name(), count);
            }
        }

        List<SaveData.OwnedCardData> cards = player.getCardHand().getCards().stream()
                .map(c -> new SaveData.OwnedCardData(
                        c.getCardId().value(),
                        c.getType().name(),
                        c.getBoughtTurnNumber()
                ))
                .toList();

        return new SaveData.PlayerData(
                player.getId().getValue(),
                player.getName(),
                player.getColor().name(),
                resources,
                player.getSupply().getSettlements(),
                player.getSupply().getLaboratories(),
                player.getSupply().getPipes(),
                player.getScore().getPublicPoints(),
                player.getScore().getHiddenPoints(),
                cards,
                player.getStats().getPlayedKnightCount(),
                player.getStats().getLongestRoadLength()
        );
    }

    private SaveData.BankData toBankData(GameState state) {
        Map<String, Integer> resources = new HashMap<>();
        for (ResourceType type : ResourceType.values()) {
            int count = state.getBank().getCount(type);
            if (count > 0) {
                resources.put(type.name(), count);
            }
        }
        return new SaveData.BankData(resources);
    }

    private SaveData.DeckData toDeckData(GameState state) {
        List<String> cardIds = state.getDevelopmentDeck().snapshot().stream()
                .map(card -> card.getId().value())
                .toList();
        return new SaveData.DeckData(state.getDevelopmentDeck().remainingCount(), cardIds);
    }

    private SavedGame fromSaveData(SaveData data) {
        Board board = reconstructBoard(data.board(), data.robberHexTileId());
        List<AbstractPlayer> players = data.players().stream()
                .map(this::reconstructPlayer)
                .toList();
        ResourceBank bank = reconstructBank(data.bank());
        DevelopmentDeck deck = reconstructDeck(data.deck());

        GameState state = new GameState(players, bank, deck);
        state.setCurrentPlayerIndex(data.currentPlayerIndex());
        state.setTurnNumber(data.turnNumber());
        state.setPhase(GamePhase.valueOf(data.phase()));
        state.setStatus(GameStatus.valueOf(data.status()));
        state.resetExperimentCardPlayedThisTurn();
        if (data.experimentCardPlayedThisTurn()) {
            state.markExperimentCardPlayedThisTurn();
        }
        state.setInitialForwardPlacementCount(data.initialForwardPlacementCount());
        state.setInitialReversePlacementCount(data.initialReversePlacementCount());
        state.setTurnTimerRemainingSeconds(data.turnTimerRemainingSeconds());
        state.setTurnTimerRunning(data.turnTimerRunning());

        if (data.longestRoadOwnerId() != null) {
            state.setLongestRoadOwnerId(new PlayerId(data.longestRoadOwnerId()));
        }
        if (data.largestArmyOwnerId() != null) {
            state.setLargestArmyOwnerId(new PlayerId(data.largestArmyOwnerId()));
        }

        // Restore buildings on board after players exist
        restoreBuildings(board, data.board());

        return new SavedGame(state, board);
    }

    private Board reconstructBoard(SaveData.BoardData boardData, String robberHexTileId) {
        Map<String, HexTile> hexTiles = new LinkedHashMap<>();
        for (var entry : boardData.hexTiles().entrySet()) {
            String hexId = entry.getKey();
            SaveData.HexTileData h = entry.getValue();
            HexCoordinate coord = new HexCoordinate(h.q(), h.r());
            TerrainType terrain = TerrainType.valueOf(h.terrain());
            TokenNumber token = h.tokenValue() != null ? new TokenNumber(h.tokenValue()) : null;
            List<IntersectionId> intersectionIds = h.intersectionIds().stream()
                    .map(IntersectionId::new)
                    .toList();
            hexTiles.put(hexId, new HexTile(hexId, coord, terrain, token, intersectionIds));
        }

        Map<IntersectionId, Intersection> intersections = new LinkedHashMap<>();
        for (var entry : boardData.intersections().entrySet()) {
            IntersectionId id = new IntersectionId(entry.getKey());
            SaveData.IntersectionData i = entry.getValue();
            Intersection intersection = new Intersection(id, new BoardPosition(i.x(), i.y()));
            for (String hexId : i.adjacentHexIds()) {
                intersection.addAdjacentHex(hexId);
            }
            intersections.put(id, intersection);
        }

        Map<PathId, com.bananarepublic.core.board.Path> paths = new LinkedHashMap<>();
        for (var entry : boardData.paths().entrySet()) {
            SaveData.PathData p = entry.getValue();
            PathId pathId = new PathId(entry.getKey());
            IntersectionId first = new IntersectionId(p.firstIntersectionId());
            IntersectionId second = new IntersectionId(p.secondIntersectionId());
            paths.put(pathId, new com.bananarepublic.core.board.Path(pathId, first, second));
        }

        List<Harbor> harbors = boardData.harbors().stream()
                .map(h -> new Harbor(
                        h.id(),
                        HarborType.valueOf(h.type()),
                        h.intersectionIds().stream().map(IntersectionId::new).toList()
                ))
                .toList();

        Robber robber = new Robber(robberHexTileId);

        return new Board(hexTiles, intersections, paths, harbors, robber);
    }

    private void restoreBuildings(Board board, SaveData.BoardData boardData) {
        for (var entry : boardData.intersections().entrySet()) {
            IntersectionId id = new IntersectionId(entry.getKey());
            SaveData.IntersectionData i = entry.getValue();
            if (i.building() == null) {
                continue;
            }
            AbstractBuilding building = BuildingFactory.create(
                    BuildType.valueOf(i.building().buildType()),
                    new PlayerId(i.building().ownerId())
            );
            Intersection intersection = board.getIntersections().get(id);
            if (intersection == null) {
                continue;
            }
            if (intersection.hasBuilding()) {
                intersection.upgradeBuilding(building);
            } else {
                intersection.placeBuilding(building);
            }
        }

        for (var entry : boardData.paths().entrySet()) {
            PathId pathId = new PathId(entry.getKey());
            SaveData.PathData p = entry.getValue();
            if (p.pipe() == null) {
                continue;
            }
            com.bananarepublic.core.board.Path path = board.getPaths().get(pathId);
            if (path == null || path.hasPipe()) {
                continue;
            }
            var pipe = (com.bananarepublic.core.building.Pipe) BuildingFactory.create(
                    BuildType.PIPE,
                    new PlayerId(p.pipe().ownerId())
            );
            path.placePipe(pipe);
        }
    }

    private AbstractPlayer reconstructPlayer(SaveData.PlayerData p) {
        int playerNumber;
        try {
            playerNumber = Integer.parseInt(p.id().substring(1));
        } catch (NumberFormatException e) {
            playerNumber = 1;
        }

        PlayerColor color = PlayerColor.valueOf(p.color());
        AbstractPlayer player = PlayerFactory.createHumanPlayer(playerNumber, p.name(), color);

        // Restore resources
        for (var entry : p.resources().entrySet()) {
            ResourceType type = ResourceType.valueOf(entry.getKey());
            int count = entry.getValue();
            for (int i = 0; i < count; i++) {
                player.addResource(type, 1);
            }
        }

        // Restore supply (adjust from full initial supply to saved remaining counts)
        int usedSettlements = com.bananarepublic.config.Constants.MAX_SETTLEMENT_SUPPLY - p.settlements();
        int usedLaboratories = com.bananarepublic.config.Constants.MAX_LABORATORY_SUPPLY - p.laboratories();
        int usedPipes = com.bananarepublic.config.Constants.MAX_PIPE_SUPPLY - p.pipes();

        for (int i = 0; i < usedSettlements; i++) {
            player.getSupply().use(com.bananarepublic.core.building.BuildType.SETTLEMENT);
        }
        for (int i = 0; i < usedLaboratories; i++) {
            player.getSupply().use(com.bananarepublic.core.building.BuildType.LABORATORY);
        }
        for (int i = 0; i < usedPipes; i++) {
            player.getSupply().use(com.bananarepublic.core.building.BuildType.PIPE);
        }

        // Restore points
        if (p.publicPoints() > 0) {
            player.getScore().addPublicPoints(p.publicPoints());
        }
        if (p.hiddenPoints() > 0) {
            player.getScore().addHiddenPoints(p.hiddenPoints());
        }

        // Restore cards in hand
        for (SaveData.OwnedCardData cardData : p.cards()) {
            CardType cardType = CardType.valueOf(cardData.cardType());
            if (cardType == CardType.PLUGIN) {
                System.err.println("Skipping plugin card from save; reload the plugin JAR before using it: " + cardData.cardId());
                continue;
            }
            ExperimentCardId cardId = new ExperimentCardId(cardData.cardId());
            ExperimentCard card = CardFactory.create(cardType, cardId);
            player.getCardHand().addCard(card, cardData.boughtTurnNumber());
        }

        // Restore stats
        if (p.playedKnightCount() > 0) {
            for (int i = 0; i < p.playedKnightCount(); i++) {
                player.getStats().recordKnightPlayed();
            }
        }
        if (p.longestRoadLength() > 0) {
            player.getStats().setLongestRoadLength(p.longestRoadLength());
        }

        return player;
    }

    private ResourceBank reconstructBank(SaveData.BankData bankData) {
        ResourceBank bank = new ResourceBank();
        for (ResourceType type : ResourceType.values()) {
            int savedCount = bankData.resources().getOrDefault(type.name(),
                    com.bananarepublic.config.ResourceConfig.INITIAL_BANK_RESOURCE_COUNT);
            int givenOut = com.bananarepublic.config.ResourceConfig.INITIAL_BANK_RESOURCE_COUNT - savedCount;
            for (int i = 0; i < givenOut; i++) {
                bank.giveResource(type, 1);
            }
        }
        return bank;
    }

    private DevelopmentDeck reconstructDeck(SaveData.DeckData deckData) {
        List<ExperimentCard> cards = new ArrayList<>();
        for (String cardIdStr : deckData.cardIds()) {
            int dashIndex = cardIdStr.lastIndexOf('-');
            if (dashIndex < 0) {
                continue;
            }
            String typeName = cardIdStr.substring(0, dashIndex);
            try {
                CardType cardType = CardType.valueOf(typeName);
                if (cardType == CardType.PLUGIN) {
                    System.err.println("Skipping plugin card from saved deck; reload the plugin JAR before using it: " + cardIdStr);
                    continue;
                }
                ExperimentCardId cardId = new ExperimentCardId(cardIdStr);
                cards.add(CardFactory.create(cardType, cardId));
            } catch (IllegalArgumentException e) {
                System.err.println("Skipping unrecognized card: " + cardIdStr);
            }
        }
        return new DevelopmentDeck(cards);
    }

    private Path ensureSuffix(Path path) {
        String name = path.getFileName().toString();
        if (!name.endsWith(SAVE_FILE_SUFFIX)) {
            return path.resolveSibling(name + SAVE_FILE_SUFFIX);
        }
        return path;
    }

    private String sanitizeFileName(String label) {
        return label.replaceAll("[^a-zA-Z0-9._-]", "_").replaceAll("_+", "_");
    }

    public record SavedGame(GameState state, Board board) {
        public SavedGame {
            Objects.requireNonNull(state, "state");
            Objects.requireNonNull(board, "board");
        }
    }
}
