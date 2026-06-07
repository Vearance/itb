package com.bananarepublic.ui.audio;

import javafx.scene.media.AudioClip;

import java.net.URL;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Small UI-level sound service for short one-shot effects.
 *
 * <p>The class intentionally isolates JavaFX media usage from controllers so
 * missing/corrupt assets or unavailable audio devices do not break gameplay.
 * Every play method is fail-safe: sound is optional polish, not game state.</p>
 */
public final class SoundEffectPlayer {
    private static final SoundEffectPlayer INSTANCE = new SoundEffectPlayer();

    private static final String MOUSE_CLICK = "/sounds/mouse-click.wav";
    private static final String DICE_ROLL = "/sounds/dice-roll.wav";
    private static final String VICTORY_CHIME = "/sounds/victory-chime.mp3";

    private final Map<String, AudioClip> clips = new ConcurrentHashMap<>();
    private volatile boolean enabled = true;

    private SoundEffectPlayer() {
    }

    public static SoundEffectPlayer getInstance() {
        return INSTANCE;
    }

    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void playMouseClick() {
        play(MOUSE_CLICK, 0.28);
    }

    public void playDiceRoll() {
        play(DICE_ROLL, 0.60);
    }

    public void playVictoryChime() {
        play(VICTORY_CHIME, 0.75);
    }

    public void playVictoryChime(double volume) {
        play(VICTORY_CHIME, volume);
    }

    private void play(String resourcePath, double volume) {
        if (!enabled) {
            return;
        }

        try {
            AudioClip clip = clips.computeIfAbsent(resourcePath, this::loadClip);
            clip.setVolume(clamp(volume));
            clip.play();
        } catch (RuntimeException ignored) {
            // Sound effects must never prevent the game from running.
        }
    }

    private AudioClip loadClip(String resourcePath) {
        URL resource = SoundEffectPlayer.class.getResource(resourcePath);
        Objects.requireNonNull(resource, "Sound effect not found: " + resourcePath);
        return new AudioClip(resource.toExternalForm());
    }

    private double clamp(double volume) {
        return Math.max(0.0, Math.min(1.0, volume));
    }
}
