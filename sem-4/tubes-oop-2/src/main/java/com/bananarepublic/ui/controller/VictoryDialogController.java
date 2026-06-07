package com.bananarepublic.ui.controller;

import com.bananarepublic.app.AppContext;
import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.service.ScoreService;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.Node;
import javafx.scene.control.Label;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

public final class VictoryDialogController {
    @FXML private Label publicPointsLabel;
    @FXML private Label secretPointsLabel;
    @FXML private VBox secretCardBox;

    @FXML private void initialize() {
        var registry = AppContext.getCurrent().serviceRegistry();
        ScoreService scoreService = getScoreService();
        int publicPoints = 0;
        int hiddenPoints = 0;
        int largestArmyPoints = 0;
        int longestRoadPoints = 0;

        secretCardBox.getChildren().clear();
        if (registry.contains(GameState.class)) {
            GameState state = registry.get(GameState.class);
            AbstractPlayer player = state.getCurrentPlayer();
            hiddenPoints = player.getScore().getHiddenPoints();
            publicPoints = player.getScore().getPublicPoints()
                    + scoreService.calculateSpecialCardPoints(state, player.getId());
            largestArmyPoints = scoreService.calculateLargestArmyPoints(state, player.getId());
            longestRoadPoints = scoreService.calculateLongestRoadPoints(state, player.getId());

            if (registry.contains(Board.class)) {
                Board board = registry.get(Board.class);
                publicPoints = scoreService.calculateVisiblePoints(state, board, player);
            }
        }

        publicPointsLabel.setText(String.valueOf(publicPoints));
        secretPointsLabel.setText(String.valueOf(hiddenPoints));
        addSecret("Victory Point Card", formatPoints(hiddenPoints));
        addSecret("Largest Army", formatPoints(largestArmyPoints));
        addSecret("Longest Road", formatPoints(longestRoadPoints));
    }

    private ScoreService getScoreService() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(ScoreService.class)) {
            return registry.get(ScoreService.class);
        }

        ScoreService service = new ScoreService();
        registry.register(ScoreService.class, service);
        return service;
    }

    private String formatPoints(int points) {
        return (points > 0 ? "+" : "") + points + " VP";
    }

    private void addSecret(String name, String points) {
        HBox row = new HBox(12, new Label(name), new Label(points));
        row.getStyleClass().add("secret-card-row");
        secretCardBox.getChildren().add(row);
    }

    @FXML private void handleCancel(ActionEvent event) { close(event); }
    @FXML private void handleConfirm(ActionEvent event) { close(event); AppContext.getCurrent().sceneManager().showGameResult(); }
    private void close(ActionEvent event) { ((Stage)((Node)event.getSource()).getScene().getWindow()).close(); }
}
