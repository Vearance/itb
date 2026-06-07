package com.bananarepublic.ui.router;

import com.bananarepublic.ui.audio.BackgroundMusicPlayer;
import com.bananarepublic.ui.audio.SoundEffectPlayer;
import javafx.application.Platform;
import javafx.geometry.Rectangle2D;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.input.MouseButton;
import javafx.scene.input.MouseEvent;
import javafx.stage.Screen;
import javafx.stage.Stage;

import java.util.Objects;

public final class SceneManager {
    private static final int DEFAULT_WIDTH = 1180;
    private static final int DEFAULT_HEIGHT = 760;
    private static final int MIN_WIDTH = 760;
    private static final int MIN_HEIGHT = 520;
    private static final int SCREEN_MARGIN = 48;

    private final Stage stage;
    private final ViewLoader viewLoader;
    private final DialogManager dialogManager;

    public SceneManager(Stage stage, ViewLoader viewLoader, DialogManager dialogManager) {
        this.stage = Objects.requireNonNull(stage, "stage");
        this.viewLoader = Objects.requireNonNull(viewLoader, "viewLoader");
        this.dialogManager = Objects.requireNonNull(dialogManager, "dialogManager");
    }

    public void showMainMenu() {
        BackgroundMusicPlayer.getInstance().stop();
        show("/fxml/main-menu.fxml", "Banana Republic");
    }

    public void showLobby() {
        BackgroundMusicPlayer.getInstance().stop();
        show("/fxml/lobby.fxml", "Banana Republic - Lobby");
    }

    public void showGame() {
        show("/fxml/game.fxml", "Banana Republic - Game");
    }

    public void showTurnTransition() {
        show("/fxml/turn-transition.fxml", "Banana Republic - Turn Transition");
    }

    public void showGameResult() {
        BackgroundMusicPlayer.getInstance().stop();
        show("/fxml/game-result.fxml", "Banana Republic - Result");
    }

    public DialogManager dialogManager() {
        return dialogManager;
    }

    private void show(String fxmlPath, String title) {
        boolean wasShowing = stage.isShowing();
        boolean wasFullScreen = stage.isFullScreen();
        boolean wasMaximized = stage.isMaximized();
        double previousX = stage.getX();
        double previousY = stage.getY();
        double previousWidth = stage.getWidth();
        double previousHeight = stage.getHeight();

        Parent root = viewLoader.load(fxmlPath);
        Rectangle2D visualBounds = Screen.getPrimary().getVisualBounds();
        double availableWidth = Math.max(1, visualBounds.getWidth() - SCREEN_MARGIN);
        double availableHeight = Math.max(1, visualBounds.getHeight() - SCREEN_MARGIN);
        double width = windowDimension(previousWidth, DEFAULT_WIDTH, availableWidth, wasShowing);
        double height = windowDimension(previousHeight, DEFAULT_HEIGHT, availableHeight, wasShowing);
        Scene scene = new Scene(root, width, height);
        installMouseClickSound(scene);
        scene.getStylesheets().add(Objects.requireNonNull(
                getClass().getResource("/css/main.css"),
                "Stylesheet not found: /css/main.css"
        ).toExternalForm());

        stage.setTitle(title);
        stage.setMinWidth(Math.min(MIN_WIDTH, width));
        stage.setMinHeight(Math.min(MIN_HEIGHT, height));
        stage.setScene(scene);
        stage.show();
        restoreWindowState(wasShowing, wasFullScreen, wasMaximized, previousX, previousY);
    }


    private void installMouseClickSound(Scene scene) {
        scene.addEventFilter(MouseEvent.MOUSE_PRESSED, event -> {
            if (event.getButton() == MouseButton.PRIMARY) {
                SoundEffectPlayer.getInstance().playMouseClick();
            }
        });
    }

    private double windowDimension(double previous, int fallback, double available, boolean wasShowing) {
        if (wasShowing && previous > 0) {
            return previous;
        }
        return Math.min(fallback, available);
    }

    private void restoreWindowState(boolean wasShowing, boolean wasFullScreen, boolean wasMaximized, double previousX, double previousY) {
        if (wasFullScreen) {
            stage.setFullScreen(true);
            Platform.runLater(() -> stage.setFullScreen(true));
            return;
        }

        if (wasMaximized) {
            stage.setMaximized(true);
            return;
        }

        if (wasShowing) {
            stage.setX(previousX);
            stage.setY(previousY);
        } else {
            stage.centerOnScreen();
        }
    }
}
