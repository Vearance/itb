package com.bananarepublic.ui.controller;

import com.bananarepublic.app.AppContext;
import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.service.ScoreService;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.collections.FXCollections;
import javafx.fxml.FXML;
import javafx.scene.control.Label;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;

public final class GameResultController {
    @FXML private Label winnerLabel;
    @FXML private Label playtimeLabel;
    @FXML private TableView<ResultRow> scoreTable;
    @FXML private TableColumn<ResultRow, String> playerColumn;
    @FXML private TableColumn<ResultRow, Number> settlementColumn;
    @FXML private TableColumn<ResultRow, Number> labColumn;
    @FXML private TableColumn<ResultRow, Number> specialColumn;
    @FXML private TableColumn<ResultRow, Number> secretColumn;
    @FXML private TableColumn<ResultRow, Number> totalColumn;

    @FXML private void initialize() {
        playerColumn.setCellValueFactory(data -> new SimpleStringProperty(data.getValue().playerName()));
        settlementColumn.setCellValueFactory(data -> new SimpleIntegerProperty(data.getValue().settlementPoints()));
        labColumn.setCellValueFactory(data -> new SimpleIntegerProperty(data.getValue().labPoints()));
        specialColumn.setCellValueFactory(data -> new SimpleIntegerProperty(data.getValue().specialPoints()));
        secretColumn.setCellValueFactory(data -> new SimpleIntegerProperty(data.getValue().secretPoints()));
        totalColumn.setCellValueFactory(data -> new SimpleIntegerProperty(data.getValue().totalPoints()));

        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(GameState.class)) {
            GameState state = registry.get(GameState.class);
            ScoreService scoreService = getScoreService();
            AbstractPlayer winner = state.getCurrentPlayer();
            winnerLabel.setText(winner.getName() + " has founded the Banana Republic!");
            if (registry.contains(Board.class)) {
                Board board = registry.get(Board.class);
                scoreTable.setItems(FXCollections.observableArrayList(state.getAllPlayers().stream()
                        .map(player -> rowOf(state, board, scoreService, player))
                        .toList()));
            } else {
                scoreTable.setItems(FXCollections.observableArrayList(state.getAllPlayers().stream()
                        .map(player -> fallbackRowOf(state, scoreService, player))
                        .toList()));
            }
        }
        playtimeLabel.setText("00:00:00");
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

    private ResultRow rowOf(GameState state, Board board, ScoreService scoreService, AbstractPlayer player) {
        int settlementPoints = scoreService.calculateSettlementPoints(board, player.getId());
        int labPoints = scoreService.calculateLaboratoryPoints(board, player.getId());
        int specialPoints = scoreService.calculateSpecialCardPoints(state, player.getId());
        int hiddenPoints = player.getScore().getHiddenPoints();
        int totalPoints = scoreService.calculateTotalPoints(state, board, player);
        return new ResultRow(player.getName(), settlementPoints, labPoints, specialPoints, hiddenPoints, totalPoints);
    }

    private ResultRow fallbackRowOf(GameState state, ScoreService scoreService, AbstractPlayer player) {
        int publicPoints = player.getScore().getPublicPoints();
        int specialPoints = scoreService.calculateSpecialCardPoints(state, player.getId());
        int hiddenPoints = player.getScore().getHiddenPoints();
        return new ResultRow(player.getName(), publicPoints, 0, specialPoints, hiddenPoints,
                publicPoints + specialPoints + hiddenPoints);
    }

    @FXML private void handleMainMenu() { AppContext.getCurrent().sceneManager().showMainMenu(); }
    @FXML private void handleViewBoard() { AppContext.getCurrent().sceneManager().showGame(); }

    public record ResultRow(String playerName, int settlementPoints, int labPoints, int specialPoints, int secretPoints, int totalPoints) { }
}
