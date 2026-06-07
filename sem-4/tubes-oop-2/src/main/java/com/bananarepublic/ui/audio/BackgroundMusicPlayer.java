package com.bananarepublic.ui.audio;

import javafx.application.Platform;
import javafx.beans.property.DoubleProperty;
import javafx.beans.property.SimpleDoubleProperty;
import javafx.scene.media.Media;
import javafx.scene.media.MediaPlayer;

import java.net.URL;
import java.util.Objects;

/**
 * UI-level background music playlist player.
 *
 * <p>Media loading is intentionally fail-safe so broken audio assets never
 * prevent the game from running.</p>
 */
public final class BackgroundMusicPlayer {
    private static final BackgroundMusicPlayer INSTANCE = new BackgroundMusicPlayer();

    private static final String[] PLAYLIST = {
            "/sounds/bgm-1.mp3",
            "/sounds/bgm-2.mp3",
            "/sounds/bgm-3.mp3"
    };
    private static final double DEFAULT_VOLUME = 0.45;

    private final DoubleProperty volume = new SimpleDoubleProperty(DEFAULT_VOLUME);

    private MediaPlayer currentPlayer;
    private int trackIndex;
    private int consecutiveFailures;
    private boolean playing;

    private BackgroundMusicPlayer() {
        volume.addListener((observable, oldValue, newValue) -> applyVolume());
    }

    public static BackgroundMusicPlayer getInstance() {
        return INSTANCE;
    }

    public DoubleProperty volumeProperty() {
        return volume;
    }

    public double getVolume() {
        return volume.get();
    }

    public void setVolume(double value) {
        runOnFxThread(() -> volume.set(clamp(value)));
    }

    public void start() {
        runOnFxThread(() -> {
            if (playing) {
                applyVolume();
                return;
            }
            playing = true;
            playFrom(trackIndex, PLAYLIST.length);
        });
    }

    public void stop() {
        runOnFxThread(() -> {
            playing = false;
            trackIndex = 0;
            consecutiveFailures = 0;
            disposeCurrentPlayer();
        });
    }

    private void playNextTrack() {
        if (!playing) {
            return;
        }
        trackIndex = (trackIndex + 1) % PLAYLIST.length;
        playFrom(trackIndex, PLAYLIST.length);
    }

    private void playFrom(int startIndex, int attemptsRemaining) {
        if (!playing) {
            return;
        }
        if (attemptsRemaining <= 0) {
            playing = false;
            consecutiveFailures = 0;
            return;
        }

        disposeCurrentPlayer();
        trackIndex = Math.floorMod(startIndex, PLAYLIST.length);

        try {
            currentPlayer = createPlayer(PLAYLIST[trackIndex]);
            currentPlayer.setVolume(clamp(volume.get()));
            currentPlayer.setOnPlaying(() -> consecutiveFailures = 0);
            currentPlayer.setOnEndOfMedia(() -> {
                consecutiveFailures = 0;
                playNextTrack();
            });
            currentPlayer.setOnError(this::handleTrackError);
            currentPlayer.play();
        } catch (RuntimeException ignored) {
            trackIndex = (trackIndex + 1) % PLAYLIST.length;
            playFrom(trackIndex, attemptsRemaining - 1);
        }
    }

    private void handleTrackError() {
        if (!playing) {
            return;
        }
        consecutiveFailures++;
        if (consecutiveFailures >= PLAYLIST.length) {
            playing = false;
            consecutiveFailures = 0;
            disposeCurrentPlayer();
            return;
        }
        playNextTrack();
    }

    private MediaPlayer createPlayer(String resourcePath) {
        URL resource = BackgroundMusicPlayer.class.getResource(resourcePath);
        Objects.requireNonNull(resource, "Background music not found: " + resourcePath);
        return new MediaPlayer(new Media(resource.toExternalForm()));
    }

    private void applyVolume() {
        if (currentPlayer != null) {
            currentPlayer.setVolume(clamp(volume.get()));
        }
    }

    private void disposeCurrentPlayer() {
        if (currentPlayer == null) {
            return;
        }
        currentPlayer.stop();
        currentPlayer.dispose();
        currentPlayer = null;
    }

    private void runOnFxThread(Runnable task) {
        if (Platform.isFxApplicationThread()) {
            task.run();
        } else {
            Platform.runLater(task);
        }
    }

    private double clamp(double value) {
        return Math.max(0.0, Math.min(1.0, value));
    }
}
