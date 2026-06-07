package com.bananarepublic.ui.controller;

import com.bananarepublic.app.AppContext;
import com.bananarepublic.core.game.GameState;
import javafx.fxml.FXML;
import javafx.scene.control.Label;

public final class TurnTransitionController {
    @FXML private Label messageLabel;

    @FXML
    private void initialize() {
        if (AppContext.getCurrent().serviceRegistry().contains(GameState.class)) {
            String name = AppContext.getCurrent().serviceRegistry().get(GameState.class).getCurrentPlayer().getName();
            messageLabel.setText("Turn completed, awaiting " + name + "'s turn. Please pass the device to the next player.");
        }
    }

    @FXML
    private void handleViewBoard() {
        AppContext.getCurrent().sceneManager().showGame();
    }
}
