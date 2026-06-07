package com.bananarepublic.ui.component;

import com.bananarepublic.ui.viewmodel.HexTileViewModel;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.Label;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.layout.StackPane;
import javafx.scene.shape.Circle;
import javafx.scene.shape.Polygon;

public final class HexTileView extends StackPane {
    private static final double DEFAULT_WIDTH = 92;
    private static final double HEIGHT_FACTOR = 1.154700538;

    private final HexTileViewModel viewModel;
    private final double tileWidth;
    private final double tileHeight;

    public HexTileView(HexTileViewModel vm) {
        this(vm, DEFAULT_WIDTH);
    }

    public HexTileView(HexTileViewModel vm, double tileWidth) {
        this.viewModel = vm;
        this.tileWidth = tileWidth;
        this.tileHeight = tileWidth * HEIGHT_FACTOR;
        getStyleClass().add("hex-stack");
        setPrefSize(this.tileWidth, this.tileHeight);
        setMinSize(this.tileWidth, this.tileHeight);
        setMaxSize(this.tileWidth, this.tileHeight);

        Polygon hex = createHexPolygon();
        hex.getStyleClass().addAll("hex-tile", terrainClass(vm.getTerrain()));

        Node tileBackground = createTileBackground(vm.getTerrain());

        Polygon hexBorder = createHexPolygon();
        hexBorder.getStyleClass().add("hex-border");
        hexBorder.setMouseTransparent(true);

        double iconSize = terrainIconSize(vm.getTerrain());
        Node terrainIcon = ResourceIconFactory.iconForTerrain(vm.getTerrain(), iconSize);
        terrainIcon.getStyleClass().add("hex-terrain-icon");
        StackPane.setAlignment(terrainIcon, Pos.TOP_CENTER);
        terrainIcon.setTranslateY(this.tileHeight * 0.12);

        Circle tokenCircle = new Circle(this.tileWidth * 0.16);
        tokenCircle.getStyleClass().add("token-circle");
        Label token = new Label(vm.getToken());
        token.getStyleClass().add(vm.getToken().equals("6") || vm.getToken().equals("8") ? "token-label-red" : "token-label");
        StackPane tokenStack = new StackPane(tokenCircle, token);
        StackPane.setAlignment(tokenStack, Pos.CENTER);
        tokenStack.setTranslateY(this.tileHeight * 0.23);

        getChildren().addAll(hex, tileBackground, hexBorder, terrainIcon, tokenStack);
    }

    private Polygon createHexPolygon() {
        return new Polygon(
                this.tileWidth, this.tileHeight * 0.25,
                this.tileWidth, this.tileHeight * 0.75,
                this.tileWidth * 0.5, this.tileHeight,
                0, this.tileHeight * 0.75,
                0, this.tileHeight * 0.25,
                this.tileWidth * 0.5, 0
        );
    }

    private Node createTileBackground(String terrain) {
        Image texture = loadTexture(terrain);
        if (texture == null) {
            StackPane empty = new StackPane();
            empty.setMouseTransparent(true);
            return empty;
        }

        ImageView textureView = new ImageView(texture);
        textureView.setFitWidth(this.tileWidth);
        textureView.setFitHeight(this.tileHeight);
        textureView.setPreserveRatio(false);
        textureView.setSmooth(false);
        textureView.setMouseTransparent(true);
        textureView.getStyleClass().add("hex-texture");

        Polygon clip = createHexPolygon();
        textureView.setClip(clip);
        return textureView;
    }

    private Image loadTexture(String terrain) {
        String path = texturePath(terrain);
        if (path == null) {
            return null;
        }
        var resource = HexTileView.class.getResource(path);
        if (resource == null) {
            return null;
        }
        return ResourceImageCache.get(path);
    }

    private String texturePath(String terrain) {
        return switch (terrain) {
            case "HUTAN" -> "/images/tile/forest.png";
            case "BUKIT" -> "/images/tile/brick.png";
            case "LADANG" -> "/images/tile/wheat.png";
            case "GUNUNG" -> "/images/tile/ore.jpg";
            case "KEBUN_PISANG" -> "/images/tile/banana.png";
            case "GURUN" -> "/images/tile/desert.png";
            default -> null;
        };
    }

    private double terrainIconSize(String terrain) {
        double baseSize = this.tileWidth * 0.34;
        return switch (terrain) {
            case "HUTAN" -> baseSize * 1.22;
            case "BUKIT" -> baseSize * 1.18;
            case "LADANG" -> baseSize * 1.45;
            case "GUNUNG" -> baseSize * 1.18;
            case "KEBUN_PISANG" -> baseSize * 1.02;
            case "GURUN" -> baseSize * 1.25;
            default -> baseSize;
        };
    }

    private String terrainClass(String terrain) {
        return switch (terrain) {
            case "HUTAN" -> "terrain-forest";
            case "BUKIT" -> "terrain-hill";
            case "LADANG" -> "terrain-field";
            case "GUNUNG" -> "terrain-mountain";
            case "KEBUN_PISANG" -> "terrain-banana";
            case "GURUN" -> "terrain-desert";
            default -> "terrain-field";
        };
    }

    public HexTileViewModel getViewModel() {
        return viewModel;
    }

    public double getTileWidth() {
        return tileWidth;
    }

    public double getTileHeight() {
        return tileHeight;
    }
}
