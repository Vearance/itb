package com.bananarepublic.ui.component;

import javafx.animation.KeyFrame;
import javafx.animation.Timeline;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.image.Image;
import javafx.scene.layout.Pane;
import javafx.util.Duration;

import java.util.List;

/**
 * Animated ocean background that repeats small pixel-art frames instead of stretching a tiny GIF.
 * This keeps the water pattern crisp and seamless even when the board is resized.
 */
public final class AnimatedTiledWaterView extends Pane {
    private static final List<String> FRAME_PATHS = List.of(
            "/images/animation/water/water-frame-0.png",
            "/images/animation/water/water-frame-1.png",
            "/images/animation/water/water-frame-2.png",
            "/images/animation/water/water-frame-3.png",
            "/images/animation/water/water-frame-4.png"
    );

    private static final double DEFAULT_TILE_SIZE = 64.0;
    private static final Duration FRAME_DURATION = Duration.millis(1000);

    private final Canvas canvas = new Canvas();
    private final List<Image> frames;
    private final Timeline timeline;
    private int frameIndex;
    private double tileSize = DEFAULT_TILE_SIZE;

    public AnimatedTiledWaterView(double width, double height) {
        this.frames = FRAME_PATHS.stream()
                .map(ResourceImageCache::get)
                .toList();

        getStyleClass().add("animated-tiled-water");
        setMouseTransparent(true);
        canvas.setMouseTransparent(true);
        getChildren().add(canvas);

        setPrefSize(width, height);
        resizeCanvas(width, height);

        widthProperty().addListener((observable, oldValue, newValue) -> redraw());
        heightProperty().addListener((observable, oldValue, newValue) -> redraw());

        this.timeline = new Timeline(new KeyFrame(FRAME_DURATION, event -> nextFrame()));
        this.timeline.setCycleCount(Timeline.INDEFINITE);
        this.timeline.play();
        redraw();
    }

    public void stop() {
        timeline.stop();
    }

    public void setTileSize(double tileSize) {
        if (tileSize <= 0) {
            throw new IllegalArgumentException("tileSize must be positive");
        }
        this.tileSize = tileSize;
        redraw();
    }

    @Override
    protected void layoutChildren() {
        super.layoutChildren();
        resizeCanvas(getWidth(), getHeight());
    }

    private void nextFrame() {
        frameIndex = (frameIndex + 1) % frames.size();
        redraw();
    }

    private void resizeCanvas(double width, double height) {
        canvas.setWidth(Math.max(1, width));
        canvas.setHeight(Math.max(1, height));
        redraw();
    }

    private void redraw() {
        if (frames.isEmpty()) {
            return;
        }
        GraphicsContext gc = canvas.getGraphicsContext2D();
        gc.setImageSmoothing(false);
        double width = canvas.getWidth();
        double height = canvas.getHeight();
        gc.clearRect(0, 0, width, height);

        Image frame = frames.get(frameIndex);
        for (double y = 0; y < height; y += tileSize) {
            for (double x = 0; x < width; x += tileSize) {
                gc.drawImage(frame, x, y, tileSize, tileSize);
            }
        }
    }
}
