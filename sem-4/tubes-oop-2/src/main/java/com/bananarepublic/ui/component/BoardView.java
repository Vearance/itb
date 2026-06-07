package com.bananarepublic.ui.component;

import java.util.List;
import java.util.Locale;
import java.util.Objects;
import java.util.function.Consumer;

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
import com.bananarepublic.core.building.AbstractBuilding;
import com.bananarepublic.core.building.BuildType;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerColor;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.ui.viewmodel.BoardViewModel;
import com.bananarepublic.ui.viewmodel.HexTileViewModel;

import javafx.geometry.Pos;
import javafx.scene.Cursor;
import javafx.scene.Node;
import javafx.scene.control.Label;
import javafx.scene.image.ImageView;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Pane;
import javafx.scene.layout.Region;
import javafx.scene.layout.StackPane;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.scene.shape.Line;
import javafx.scene.shape.Polygon;
import javafx.scene.shape.Rectangle;
import javafx.scene.shape.StrokeLineCap;

public final class BoardView extends Pane {
    private static final double DEFAULT_VIEW_WIDTH = 880;
    private static final double DEFAULT_VIEW_HEIGHT = 610;
    private static final double MIN_VIEW_WIDTH = 320;
    private static final double MIN_VIEW_HEIGHT = 220;
    private static final double POINT_RADIUS = 7;
    private static final String CLOUDS_GIF = "/images/animation/moving-clouds.gif";
    private static final String NIMON_UNGU_IMAGE = "/images/icon/nimon-ungu.png";

    private Board board;
    private List<AbstractPlayer> players = List.of();
    private BoardViewModel legacyViewModel;
    private Consumer<IntersectionId> intersectionHandler = id -> { };
    private Consumer<PathId> pathHandler = id -> { };
    private Consumer<String> hexTileHandler = id -> { };
    private IntersectionId pendingSettlementId;
    private PlayerId pendingSettlementOwnerId;
    private boolean robberSelectionMode;
    private AnimatedTiledWaterView activeWaterView;

    public BoardView() {
        getStyleClass().add("board-view");
        setPrefSize(DEFAULT_VIEW_WIDTH, DEFAULT_VIEW_HEIGHT);
        setMinSize(MIN_VIEW_WIDTH, MIN_VIEW_HEIGHT);
        setMaxSize(Double.MAX_VALUE, Double.MAX_VALUE);
        widthProperty().addListener((observable, oldValue, newValue) -> refresh());
        heightProperty().addListener((observable, oldValue, newValue) -> refresh());
    }

    public void setViewModel(BoardViewModel vm) {
        this.legacyViewModel = vm;
        redrawLegacy();
    }

    public void setBoard(Board board, List<AbstractPlayer> players) {
        this.board = Objects.requireNonNull(board, "board");
        this.players = players == null ? List.of() : List.copyOf(players);
        redrawBoard();
    }

    public BoardViewModel getViewModel() {
        return legacyViewModel;
    }

    public void setOnIntersectionSelected(Consumer<IntersectionId> handler) {
        this.intersectionHandler = handler == null ? id -> { } : handler;
    }

    public void setOnPathSelected(Consumer<PathId> handler) {
        this.pathHandler = handler == null ? id -> { } : handler;
    }

    public void setOnHexTileSelected(Consumer<String> handler) {
        this.hexTileHandler = handler == null ? id -> { } : handler;
    }

    public void setRobberSelectionMode(boolean robberSelectionMode) {
        this.robberSelectionMode = robberSelectionMode;
        refresh();
    }

    public void setPendingSettlement(IntersectionId intersectionId, PlayerId ownerId) {
        this.pendingSettlementId = intersectionId;
        this.pendingSettlementOwnerId = ownerId;
        refresh();
    }

    public void clearPendingSettlement() {
        this.pendingSettlementId = null;
        this.pendingSettlementOwnerId = null;
        refresh();
    }

    public void refresh() {
        if (board != null) {
            redrawBoard();
        } else {
            redrawLegacy();
        }
    }

    private void redrawBoard() {
        stopActiveWaterView();
        getChildren().clear();
        if (board == null) {
            return;
        }

        double viewWidth = viewWidth();
        double viewHeight = viewHeight();
        BoundsTransform transform = transformForBoard(board, viewWidth, viewHeight);
        drawOceanFrame(viewWidth, viewHeight);
        drawSandShore(transform);
        drawHexTiles(transform);
        drawHarbors(transform);
        drawPipes(transform);
        drawIntersections(transform);
        drawRobber(transform);
        drawRobberTargetHotspots(transform);
        drawCloudOverlays(viewWidth, viewHeight);
    }

    private void drawHexTiles(BoundsTransform transform) {
        for (HexTile tile : board.getHexTiles().values()) {
            BoardPosition center = hexCenter(tile.getCoordinate());
            double cx = transform.x(center.x());
            double cy = transform.y(center.y());
            HexTileView view = new HexTileView(new HexTileViewModel(tile), Math.sqrt(3) * 100.0 * transform.scale());
            view.setLayoutX(cx - view.getTileWidth() / 2.0);
            view.setLayoutY(cy - view.getTileHeight() / 2.0);
            if (robberSelectionMode) {
                view.getStyleClass().add(tile.getId().equals(board.getRobber().getHexTileId()) ? "robber-current-tile" : "robber-target-tile");
                view.setCursor(Cursor.HAND);
            }
            getChildren().add(view);
        }
    }

    private void drawRobberTargetHotspots(BoundsTransform transform) {
        if (!robberSelectionMode) {
            return;
        }

        double tileWidth = Math.sqrt(3) * 100.0 * transform.scale();
        double tileHeight = tileWidth * 1.154700538;
        for (HexTile tile : board.getHexTiles().values()) {
            BoardPosition center = hexCenter(tile.getCoordinate());
            double x = transform.x(center.x()) - tileWidth / 2.0;
            double y = transform.y(center.y()) - tileHeight / 2.0;
            Polygon hotspot = createHexHotspot(x, y, tileWidth, tileHeight);
            hotspot.getStyleClass().add("robber-target-hotspot");
            hotspot.setFill(Color.TRANSPARENT);
            hotspot.setStroke(Color.TRANSPARENT);
            hotspot.setCursor(Cursor.HAND);
            hotspot.setOnMouseClicked(event -> {
                hexTileHandler.accept(tile.getId());
                event.consume();
            });
            getChildren().add(hotspot);
        }
    }

    private Polygon createHexHotspot(double x, double y, double tileWidth, double tileHeight) {
        return new Polygon(
                x + tileWidth, y + tileHeight * 0.25,
                x + tileWidth, y + tileHeight * 0.75,
                x + tileWidth * 0.5, y + tileHeight,
                x, y + tileHeight * 0.75,
                x, y + tileHeight * 0.25,
                x + tileWidth * 0.5, y
        );
    }

    private void drawOceanFrame(double viewWidth, double viewHeight) {
        activeWaterView = new AnimatedTiledWaterView(viewWidth, viewHeight);
        activeWaterView.setLayoutX(0);
        activeWaterView.setLayoutY(0);
        activeWaterView.setOpacity(0.94);
        activeWaterView.setTileSize(64);
        getChildren().add(activeWaterView);

        Rectangle oceanBorder = new Rectangle(0, 0, viewWidth, viewHeight);
        oceanBorder.getStyleClass().add("ocean-frame");
        oceanBorder.setMouseTransparent(true);
        getChildren().add(oceanBorder);
    }

    private void stopActiveWaterView() {
        if (activeWaterView != null) {
            activeWaterView.stop();
            activeWaterView = null;
        }
    }

    private void drawPipes(BoundsTransform transform) {
        for (Path path : board.getPaths().values()) {
            BoardPosition a = board.getIntersections().get(path.getFirstIntersectionId()).getPosition();
            BoardPosition b = board.getIntersections().get(path.getSecondIntersectionId()).getPosition();
            double x1 = transform.x(a.x());
            double y1 = transform.y(a.y());
            double x2 = transform.x(b.x());
            double y2 = transform.y(b.y());

            if (path.hasPipe()) {
                path.getPipe().ifPresent(pipe -> drawBuiltPipe(
                        path.getId(),
                        x1,
                        y1,
                        x2,
                        y2,
                        pieceImagePath("road", playerColorOf(pipe.getOwnerId())),
                        transform.scale()));
                continue;
            }

            Line edge = new Line(x1, y1, x2, y2);
            edge.getStyleClass().add("build-path-hotspot");
            edge.setOnMouseClicked(event -> pathHandler.accept(path.getId()));
            getChildren().add(edge);
        }
    }

    private void drawIntersections(BoundsTransform transform) {
        for (Intersection intersection : board.getIntersections().values()) {
            double x = transform.x(intersection.getPosition().x());
            double y = transform.y(intersection.getPosition().y());
            if (intersection.hasBuilding()) {
                drawBuilding(intersection, x, y, transform.scale());
            } else if (intersection.getId().equals(pendingSettlementId) && pendingSettlementOwnerId != null) {
                drawSettlementMarker(intersection.getId(), pendingSettlementOwnerId, x, y, transform.scale());
            } else {
                Circle hotspot = new Circle(x, y, POINT_RADIUS);
                hotspot.getStyleClass().add("intersection-hotspot");
                hotspot.setOnMouseClicked(event -> intersectionHandler.accept(intersection.getId()));
                getChildren().add(hotspot);
            }
        }
    }

    private void drawBuilding(Intersection intersection, double x, double y, double scale) {
        AbstractBuilding building = intersection.getBuilding().orElseThrow();
        if (building.getBuildType() == BuildType.LABORATORY) {
            double height = Math.max(32, 42 * scale);
            drawPieceMarker(
                    intersection.getId(),
                    pieceImagePath("lab", playerColorOf(building.getOwnerId())),
                    x,
                    y,
                    height * 0.79,
                    height);
            return;
        }

        drawSettlementMarker(intersection.getId(), building.getOwnerId(), x, y, scale);
    }

    private void drawSettlementMarker(IntersectionId intersectionId, PlayerId ownerId, double x, double y, double scale) {
        double height = Math.max(30, 40 * scale);
        drawPieceMarker(intersectionId, pieceImagePath("house", playerColorOf(ownerId)), x, y, height * 0.84, height);
    }

    private void drawPieceMarker(
            IntersectionId intersectionId,
            String imagePath,
            double x,
            double y,
            double width,
            double height) {
        ImageView image = staticImageView(imagePath, width, height);
        image.getStyleClass().add("piece-image");
        image.setMouseTransparent(true);

        StackPane marker = new StackPane(image);
        marker.getStyleClass().add("piece-marker");
        marker.setMinSize(width, height);
        marker.setPrefSize(width, height);
        marker.setMaxSize(width, height);
        marker.setLayoutX(x - width / 2.0);
        marker.setLayoutY(y - height / 2.0);
        marker.setCursor(Cursor.HAND);
        marker.setPickOnBounds(true);
        marker.setOnMouseClicked(event -> {
            intersectionHandler.accept(intersectionId);
            event.consume();
        });
        getChildren().add(marker);
    }

    private void drawBuiltPipe(
            PathId pathId,
            double x1,
            double y1,
            double x2,
            double y2,
            String imagePath,
            double scale) {
        double length = Math.hypot(x2 - x1, y2 - y1);
        double height = Math.max(13, 18 * scale);
        double width = Math.max(32, length * 0.82);
        double midpointX = (x1 + x2) / 2.0;
        double midpointY = (y1 + y2) / 2.0;

        Line hitbox = new Line(x1, y1, x2, y2);
        hitbox.getStyleClass().add("road-piece-hitbox");
        hitbox.setStroke(Color.TRANSPARENT);
        hitbox.setStrokeWidth(height + Math.max(8, 8 * scale));
        hitbox.setStrokeLineCap(StrokeLineCap.ROUND);
        hitbox.setCursor(Cursor.HAND);
        hitbox.setOnMouseClicked(event -> {
            pathHandler.accept(pathId);
            event.consume();
        });
        getChildren().add(hitbox);

        ImageView road = staticImageView(imagePath, width, height);
        road.getStyleClass().add("road-piece-image");
        road.setLayoutX(midpointX - width / 2.0);
        road.setLayoutY(midpointY - height / 2.0);
        road.setRotate(Math.toDegrees(Math.atan2(y2 - y1, x2 - x1)));
        road.setCursor(Cursor.HAND);
        road.setPickOnBounds(true);
        road.setOnMouseClicked(event -> {
            pathHandler.accept(pathId);
            event.consume();
        });
        getChildren().add(road);
    }

    private void drawHarbors(BoundsTransform transform) {
        for (Harbor harbor : board.getHarbors()) {
            List<BoardPosition> positions = harbor.getIntersectionIds().stream()
                    .map(board.getIntersections()::get)
                    .filter(Objects::nonNull)
                    .map(Intersection::getPosition)
                    .toList();
            if (positions.size() < 2) {
                continue;
            }

            BoardPosition first = positions.get(0);
            BoardPosition second = positions.get(1);
            double x1 = transform.x(first.x());
            double y1 = transform.y(first.y());
            double x2 = transform.x(second.x());
            double y2 = transform.y(second.y());

            Line coastalEdge = new Line(x1, y1, x2, y2);
            coastalEdge.getStyleClass().add("harbor-edge");
            getChildren().add(coastalEdge);

            BoardPosition midpoint = new BoardPosition((first.x() + second.x()) / 2.0, (first.y() + second.y()) / 2.0);
            BoardPosition labelPoint = moveAwayFromCenter(midpoint, 82);
            double labelX = transform.x(labelPoint.x());
            double labelY = transform.y(labelPoint.y());
            Line firstConnector = new Line(x1, y1, labelX, labelY);
            firstConnector.getStyleClass().add("harbor-connector");
            getChildren().add(firstConnector);

            Line secondConnector = new Line(x2, y2, labelX, labelY);
            secondConnector.getStyleClass().add("harbor-connector");
            getChildren().add(secondConnector);

            StackPane marker = createHarborMarker(harbor.getType());
            marker.setLayoutX(labelX - 33);
            marker.setLayoutY(labelY - 42);
            getChildren().add(marker);
        }
    }

    private void drawSandShore(BoundsTransform transform) {
        double shoreWidth = Math.max(18, 30 * transform.scale());

        for (HexTile tile : board.getHexTiles().values()) {
            List<IntersectionId> intersectionIds = tile.getIntersectionIds();
            for (int i = 0; i < intersectionIds.size(); i++) {
                IntersectionId first = intersectionIds.get(i);
                IntersectionId second = intersectionIds.get((i + 1) % intersectionIds.size());
                if (!isCoastalEdge(first, second)) {
                    continue;
                }

                Intersection firstIntersection = board.getIntersections().get(first);
                Intersection secondIntersection = board.getIntersections().get(second);
                if (firstIntersection == null || secondIntersection == null) {
                    continue;
                }

                BoardPosition a = firstIntersection.getPosition();
                BoardPosition b = secondIntersection.getPosition();
                Line shore = new Line(transform.x(a.x()), transform.y(a.y()), transform.x(b.x()), transform.y(b.y()));
                shore.getStyleClass().add("sand-shore");
                shore.setStrokeWidth(shoreWidth);
                shore.setStrokeLineCap(StrokeLineCap.ROUND);
                shore.setMouseTransparent(true);
                getChildren().add(shore);
            }
        }
    }

    private boolean isCoastalEdge(IntersectionId first, IntersectionId second) {
        Intersection a = board.getIntersections().get(first);
        Intersection b = board.getIntersections().get(second);
        if (a == null || b == null) {
            return false;
        }
        long touchingHexes = a.getAdjacentHexIds().stream()
                .filter(b.getAdjacentHexIds()::contains)
                .count();
        return touchingHexes == 1;
    }

    private void drawRobber(BoundsTransform transform) {
        board.getHexTile(board.getRobber().getHexTileId()).ifPresent(tile -> {
            BoardPosition center = hexCenter(tile.getCoordinate());
            double x = transform.x(center.x());
            double y = transform.y(center.y());
            double tileWidth = Math.sqrt(3) * 100.0 * transform.scale();
            double tileHeight = tileWidth * 1.154700538;
            double imageSize = Math.max(26, 38 * transform.scale());
            double frameSize = Math.max(34, 46 * transform.scale());

            ImageView nimon = animatedImageView(NIMON_UNGU_IMAGE, imageSize, imageSize);
            nimon.getStyleClass().add("robber-image");
            nimon.setPreserveRatio(true);

            Rectangle frame = new Rectangle(frameSize, frameSize);
            frame.getStyleClass().add("robber-piece-frame");
            StackPane piece = new StackPane(frame, nimon);
            piece.getStyleClass().add("robber-piece");
            piece.setMouseTransparent(true);

            double pieceX = x - tileWidth * 0.28 - frameSize / 2.0;
            double pieceY = y + tileHeight * 0.07 - frameSize / 2.0;
            piece.setLayoutX(pieceX);
            piece.setLayoutY(pieceY);
            getChildren().add(piece);
        });
    }

    private void drawCloudOverlays(double viewWidth, double viewHeight) {
        double cloudWidth = Math.max(120, Math.min(230, viewWidth * 0.19));
        double cloudHeight = cloudWidth * 0.62;

        ImageView bottomLeftCloud = animatedImageView(CLOUDS_GIF, cloudWidth, cloudHeight);
        bottomLeftCloud.getStyleClass().add("cloud-overlay");
        bottomLeftCloud.setLayoutX(Math.max(18, viewWidth * 0.035));
        bottomLeftCloud.setLayoutY(Math.max(18, viewHeight - cloudHeight - viewHeight * 0.055));
        bottomLeftCloud.setOpacity(0.72);

        ImageView topRightCloud = animatedImageView(CLOUDS_GIF, cloudWidth * 0.9, cloudHeight * 0.9);
        topRightCloud.getStyleClass().add("cloud-overlay");
        topRightCloud.setLayoutX(Math.max(18, viewWidth - cloudWidth * 0.95 - viewWidth * 0.045));
        topRightCloud.setLayoutY(Math.max(18, viewHeight * 0.035));
        topRightCloud.setOpacity(0.66);

        getChildren().addAll(bottomLeftCloud, topRightCloud);
    }

    private ImageView animatedImageView(String resourcePath, double fitWidth, double fitHeight) {
        ImageView imageView = new ImageView(ResourceImageCache.get(resourcePath));
        imageView.setFitWidth(fitWidth);
        imageView.setFitHeight(fitHeight);
        imageView.setPreserveRatio(false);
        imageView.setSmooth(false);
        imageView.setMouseTransparent(true);
        return imageView;
    }

    private ImageView staticImageView(String resourcePath, double fitWidth, double fitHeight) {
        ImageView imageView = new ImageView(ResourceImageCache.get(resourcePath));
        imageView.setFitWidth(fitWidth);
        imageView.setFitHeight(fitHeight);
        imageView.setPreserveRatio(false);
        imageView.setSmooth(true);
        return imageView;
    }

    private BoardPosition moveAwayFromCenter(BoardPosition p, double distance) {
        double len = Math.max(1, Math.hypot(p.x(), p.y()));
        return new BoardPosition(p.x() + p.x() / len * distance, p.y() + p.y() / len * distance);
    }

    private StackPane createHarborMarker(HarborType type) {
        ImageView lighthouse = new ImageView(ResourceImageCache.get("/images/icon/lighthouse-harbor.png"));
        lighthouse.getStyleClass().add("harbor-lighthouse-image");
        lighthouse.setFitWidth(48);
        lighthouse.setFitHeight(48);
        lighthouse.setPreserveRatio(true);
        lighthouse.setSmooth(false);
        lighthouse.setMouseTransparent(true);

        Label ratio = new Label(ratioText(type));
        ratio.getStyleClass().add("harbor-ratio-text");

        Node harborIcon = ResourceIconFactory.iconForHarbor(type, 17);
        HBox content = new HBox(4, ratio, harborIcon);
        content.getStyleClass().add("harbor-lighthouse-content");
        content.setAlignment(Pos.CENTER);
        content.setMaxSize(Region.USE_PREF_SIZE, Region.USE_PREF_SIZE);
        content.setMouseTransparent(true);

        StackPane marker = new StackPane(lighthouse, content);
        StackPane.setAlignment(lighthouse, Pos.TOP_CENTER);
        StackPane.setAlignment(content, Pos.BOTTOM_CENTER);
        marker.getStyleClass().add("harbor-lighthouse-marker");
        marker.setMinSize(66, 62);
        marker.setPrefSize(66, 62);
        marker.setMaxSize(66, 62);
        marker.setMouseTransparent(true);
        return marker;
    }

    private String ratioText(HarborType type) {
        return type == HarborType.UMUM_3_1 ? "3:1" : "2:1";
    }

    private String pieceImagePath(String pieceName, PlayerColor color) {
        return "/images/piece/" + pieceName + "-" + color.name().toLowerCase(Locale.ROOT) + ".png";
    }

    private PlayerColor playerColorOf(PlayerId playerId) {
        return players.stream()
                .filter(player -> player.getId().equals(playerId))
                .findFirst()
                .map(AbstractPlayer::getColor)
                .orElse(PlayerColor.WHITE);
    }

    private BoundsTransform transformForBoard(Board board, double viewWidth, double viewHeight) {
        List<BoardPosition> positions = board.getIntersections().values().stream().map(Intersection::getPosition).toList();
        double minX = positions.stream().mapToDouble(BoardPosition::x).min().orElse(-300);
        double maxX = positions.stream().mapToDouble(BoardPosition::x).max().orElse(300);
        double minY = positions.stream().mapToDouble(BoardPosition::y).min().orElse(-260);
        double maxY = positions.stream().mapToDouble(BoardPosition::y).max().orElse(260);
        double horizontalMargin = Math.min(95, viewWidth * 0.11);
        double verticalMargin = Math.min(60, viewHeight * 0.10);
        double availableW = Math.max(1, viewWidth - horizontalMargin * 2);
        double availableH = Math.max(1, viewHeight - verticalMargin * 2);
        double scale = Math.min(availableW / Math.max(1, maxX - minX), availableH / Math.max(1, maxY - minY));
        double offsetX = horizontalMargin + (availableW - (maxX - minX) * scale) / 2.0 - minX * scale;
        double offsetY = verticalMargin + (availableH - (maxY - minY) * scale) / 2.0 - minY * scale;
        return new BoundsTransform(scale, offsetX, offsetY);
    }

    private BoardPosition hexCenter(HexCoordinate coordinate) {
        double x = 100.0 * Math.sqrt(3) * (coordinate.q() + coordinate.r() / 2.0);
        double y = 100.0 * 1.5 * coordinate.r();
        return new BoardPosition(x, y);
    }

    private void redrawLegacy() {
        getChildren().clear();
        double viewWidth = viewWidth();
        double viewHeight = viewHeight();
        drawOceanFrame(viewWidth, viewHeight);
        if (legacyViewModel == null) {
            return;
        }
        int[] rows = {3, 4, 5, 4, 3};
        int index = 0;
        double startY = 88;
        double startX = 202;
        double hexStepX = 96;
        double hexStepY = 84;
        double rowOffset = 48;
        double scale = Math.min(viewWidth / DEFAULT_VIEW_WIDTH, viewHeight / DEFAULT_VIEW_HEIGHT);
        double offsetX = (viewWidth - DEFAULT_VIEW_WIDTH * scale) / 2.0;
        double offsetY = (viewHeight - DEFAULT_VIEW_HEIGHT * scale) / 2.0;

        for (int r = 0; r < rows.length; r++) {
            for (int c = 0; c < rows[r] && index < legacyViewModel.getHexTiles().size(); c++) {
                HexTileViewModel tile = legacyViewModel.getHexTiles().get(index++);
                HexTileView view = new HexTileView(tile, 76 * scale);
                view.setLayoutX(offsetX + (startX + c * hexStepX + (5 - rows[r]) * rowOffset) * scale);
                view.setLayoutY(offsetY + (startY + r * hexStepY) * scale);
                getChildren().add(view);
            }
        }
        drawCloudOverlays(viewWidth, viewHeight);
    }

    private double viewWidth() {
        return effectiveDimension(getWidth(), getPrefWidth(), DEFAULT_VIEW_WIDTH);
    }

    private double viewHeight() {
        return effectiveDimension(getHeight(), getPrefHeight(), DEFAULT_VIEW_HEIGHT);
    }

    private double effectiveDimension(double actual, double preferred, double fallback) {
        if (actual > 0) {
            return actual;
        }
        return preferred > 0 ? preferred : fallback;
    }

    private record BoundsTransform(double scale, double offsetX, double offsetY) {
        double x(double sourceX) { return sourceX * scale + offsetX; }
        double y(double sourceY) { return sourceY * scale + offsetY; }
    }
}
