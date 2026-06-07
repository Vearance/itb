package com.bananarepublic.ui.component;

import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.geometry.Pos;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Pane;
import javafx.scene.layout.Region;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.scene.shape.Rectangle;

public final class DiceView extends HBox {
    private static final double DIE_SIZE = 44;
    private static final double PIP_RADIUS = 3.8;

    private final IntegerProperty first = new SimpleIntegerProperty(1);
    private final IntegerProperty second = new SimpleIntegerProperty(1);
    private final Pane firstFace = new Pane();
    private final Pane secondFace = new Pane();

    public DiceView() {
        getStyleClass().add("dice-view");
        setAlignment(Pos.CENTER);
        setMaxSize(Region.USE_PREF_SIZE, Region.USE_PREF_SIZE);
        setPickOnBounds(false);
        getChildren().addAll(firstFace, secondFace);
        first.addListener((observable, oldValue, newValue) -> render(firstFace, newValue.intValue()));
        second.addListener((observable, oldValue, newValue) -> render(secondFace, newValue.intValue()));
        render(firstFace, first.get());
        render(secondFace, second.get());
    }

    public void setRoll(int one, int two) {
        first.set(clampDie(one));
        second.set(clampDie(two));
    }

    public int getTotal() {
        return first.get() + second.get();
    }

    private int clampDie(int value) {
        return Math.max(1, Math.min(6, value));
    }

    private void render(Pane face, int value) {
        face.getChildren().clear();
        face.setMinSize(DIE_SIZE, DIE_SIZE);
        face.setPrefSize(DIE_SIZE, DIE_SIZE);
        face.setMaxSize(DIE_SIZE, DIE_SIZE);

        Image asset = loadAsset(value);
        if (asset != null) {
            ImageView imageView = new ImageView(asset);
            imageView.setFitWidth(DIE_SIZE);
            imageView.setFitHeight(DIE_SIZE);
            imageView.setPreserveRatio(true);
            face.getChildren().add(imageView);
            return;
        }

        Rectangle body = new Rectangle(0, 0, DIE_SIZE, DIE_SIZE);
        body.getStyleClass().add("die-face");
        face.getChildren().add(body);

        switch (value) {
            case 1 -> pip(face, 0.5, 0.5);
            case 2 -> {
                pip(face, 0.3, 0.3);
                pip(face, 0.7, 0.7);
            }
            case 3 -> {
                pip(face, 0.3, 0.3);
                pip(face, 0.5, 0.5);
                pip(face, 0.7, 0.7);
            }
            case 4 -> {
                pip(face, 0.3, 0.3);
                pip(face, 0.7, 0.3);
                pip(face, 0.3, 0.7);
                pip(face, 0.7, 0.7);
            }
            case 5 -> {
                pip(face, 0.3, 0.3);
                pip(face, 0.7, 0.3);
                pip(face, 0.5, 0.5);
                pip(face, 0.3, 0.7);
                pip(face, 0.7, 0.7);
            }
            case 6 -> {
                pip(face, 0.3, 0.25);
                pip(face, 0.7, 0.25);
                pip(face, 0.3, 0.5);
                pip(face, 0.7, 0.5);
                pip(face, 0.3, 0.75);
                pip(face, 0.7, 0.75);
            }
            default -> throw new IllegalArgumentException("Invalid die value: " + value);
        }
    }

    private Image loadAsset(int value) {
        String path = "/images/dice/die-" + value + ".png";
        try {
            var resource = getClass().getResource(path);
            return resource == null ? null : ResourceImageCache.get(path);
        } catch (RuntimeException ignored) {
            return null;
        }
    }

    private void pip(Pane face, double xRatio, double yRatio) {
        Circle pip = new Circle(DIE_SIZE * xRatio, DIE_SIZE * yRatio, PIP_RADIUS);
        pip.getStyleClass().add("die-pip");
        pip.setFill(Color.web("#2d2a25"));
        face.getChildren().add(pip);
    }
}
