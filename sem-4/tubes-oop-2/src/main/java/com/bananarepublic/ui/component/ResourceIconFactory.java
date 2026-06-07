package com.bananarepublic.ui.component;

import java.util.Locale;
import java.util.Map;

import com.bananarepublic.core.board.HarborType;
import com.bananarepublic.core.player.PlayerColor;
import com.bananarepublic.core.resource.ResourceType;

import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.Label;
import javafx.scene.image.ImageView;
import javafx.scene.layout.StackPane;

/**
 * Small UI factory for resource/terrain icons. Keeping this in one place avoids
 * duplicating image lookup rules across BoardView, HexTileView, and dialogs.
 */
public final class ResourceIconFactory {
    private static final Map<String, String> ICONS = Map.of(
            "WOOD", "/images/resource/pixel-wood.png",
            "BRICK", "/images/resource/pixel-bricks.png",
            "WHEAT", "/images/resource/pixel-wheat.png",
            "ORE", "/images/resource/pixel-ore.png",
            "BANANA", "/images/resource/pixel-banana.png",
            "DESERT", "/images/resource/pixel-desert.png"
    );

    private ResourceIconFactory() {
    }

    public static ImageView iconForResource(ResourceType type, double size) {
        return iconForKey(display(type), size);
    }

    public static Node iconForHarbor(HarborType type, double size) {
        ResourceType resource = harborResource(type);
        if (resource != null) {
            return iconForResource(resource, size);
        }
        Label wildcard = new Label("?");
        wildcard.getStyleClass().add("harbor-wildcard-icon");
        wildcard.setMinSize(size, size);
        wildcard.setPrefSize(size, size);
        wildcard.setMaxSize(size, size);
        wildcard.setAlignment(Pos.CENTER);
        return wildcard;
    }

    public static ImageView iconForTerrain(String terrain, double size) {
        return iconForKey(terrainToKey(terrain), size);
    }

    public static ImageView settingsIcon(double size) {
        return imageView("/images/icon/settings.png", size);
    }

    public static ImageView iconForPiece(String pieceName, PlayerColor color, double fitWidth, double fitHeight) {
        String colorName = color == null ? "white" : color.name().toLowerCase(Locale.ROOT);
        return imageView("/images/piece/" + pieceName + "-" + colorName + ".png", fitWidth, fitHeight, true);
    }

    public static String display(ResourceType type) {
        return switch (type) {
            case KAYU -> "WOOD";
            case BATU_BATA -> "BRICK";
            case GANDUM -> "WHEAT";
            case BIJIH -> "ORE";
            case PISANG -> "BANANA";
        };
    }

    private static ImageView iconForKey(String key, double size) {
        String normalized = key == null ? "" : key.toUpperCase(Locale.ROOT);
        String path = ICONS.get(normalized);
        if (path == null) {
            return fallbackIcon(size);
        }
        return imageView(path, size);
    }

    private static ImageView imageView(String path, double size) {
        return imageView(path, size, size, false);
    }

    private static ImageView imageView(String path, double fitWidth, double fitHeight, boolean smooth) {
        var resource = ResourceIconFactory.class.getResource(path);
        if (resource == null) {
            return fallbackIcon(Math.max(fitWidth, fitHeight));
        }
        ImageView view = new ImageView(ResourceImageCache.get(path));
        view.setFitWidth(fitWidth);
        view.setFitHeight(fitHeight);
        view.setPreserveRatio(true);
        view.setSmooth(smooth);
        view.setMouseTransparent(true);
        return view;
    }

    private static ImageView fallbackIcon(double size) {
        StackPane placeholder = new StackPane();
        placeholder.setMinSize(size, size);
        placeholder.setPrefSize(size, size);
        placeholder.getStyleClass().add("resource-icon-fallback");
        return new ImageView();
    }

    private static String terrainToKey(String terrain) {
        return switch (terrain) {
            case "HUTAN" -> "WOOD";
            case "BUKIT" -> "BRICK";
            case "LADANG" -> "WHEAT";
            case "GUNUNG" -> "ORE";
            case "KEBUN_PISANG" -> "BANANA";
            case "GURUN" -> "DESERT";
            default -> terrain == null ? "" : terrain.replace('_', ' ');
        };
    }

    private static ResourceType harborResource(HarborType type) {
        return switch (type) {
            case KAYU_2_TO_1 -> ResourceType.KAYU;
            case BATU_BATA_2_TO_1 -> ResourceType.BATU_BATA;
            case GANDUM_2_TO_1 -> ResourceType.GANDUM;
            case BIJIH_2_TO_1 -> ResourceType.BIJIH;
            case PISANG_2_TO_1 -> ResourceType.PISANG;
            case UMUM_3_1 -> null;
        };
    }
}
