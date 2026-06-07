package com.bananarepublic.ui.controller;

import com.bananarepublic.app.AppContext;
import com.bananarepublic.service.PersistenceService;
import com.bananarepublic.ui.util.AlertUtil;
import com.bananarepublic.ui.viewmodel.GameViewModel;
import javafx.application.Platform;
import javafx.fxml.FXML;
import javafx.stage.FileChooser;

import java.io.File;

public final class MainMenuController {
    @FXML
    private void handleNewGame() {
        AppContext.getCurrent().sceneManager().showLobby();
    }

    @FXML
    private void handleLoadGame() {
        FileChooser chooser = new FileChooser();
        chooser.setTitle("Muat Permainan yang Disimpan");
        chooser.getExtensionFilters().add(new FileChooser.ExtensionFilter(
                "Banana Republic Save", "*.banana-republic.json"
        ));

        File file = chooser.showOpenDialog(null);
        if (file == null) {
            return;
        }

        try {
            PersistenceService.SavedGame saved = new PersistenceService().load(file.toPath());
            var registry = AppContext.getCurrent().serviceRegistry();

            // Clear any stale state then register the restored objects
            registry.register(com.bananarepublic.core.game.GameState.class, saved.state());
            registry.register(com.bananarepublic.core.board.Board.class, saved.board());
            registry.register(GameViewModel.class, new GameViewModel());

            String summary = String.format(
                    "Giliran %s (Turn %d) — %d pemain",
                    saved.state().getCurrentPlayer().getName(),
                    saved.state().getTurnNumber(),
                    saved.state().getPlayers().size()
            );

            if (AlertUtil.confirm("Load Game", "Permainan berhasil dimuat.\n" + summary + "\n\nLanjutkan?")) {
                AppContext.getCurrent().sceneManager().showGame();
            }
        } catch (Exception e) {
            AlertUtil.error("Load Game", "Gagal memuat permainan:\n" + e.getMessage());
        }
    }

    @FXML
    private void handleExit() {
        Platform.exit();
    }
}
