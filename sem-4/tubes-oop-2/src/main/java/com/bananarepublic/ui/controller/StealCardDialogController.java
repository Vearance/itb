package com.bananarepublic.ui.controller;

import com.bananarepublic.app.AppContext;
import com.bananarepublic.app.ServiceRegistry;
import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.service.RobberService;
import com.bananarepublic.ui.util.AlertUtil;
import com.bananarepublic.validator.RobberValidator;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.Node;
import javafx.scene.control.Label;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

public final class StealCardDialogController {
    @FXML private VBox targetBox;

    private final RobberValidator robberValidator = new RobberValidator();
    private final List<AbstractPlayer> validTargets = new ArrayList<>();
    private AbstractPlayer selectedPlayer;
    private HBox selectedRow;

    @FXML
    private void initialize() {
        targetBox.getChildren().clear();
        Optional<GameState> stateOpt = resolveGameState();
        Optional<Board> boardOpt = resolveBoard();
        if (stateOpt.isEmpty() || boardOpt.isEmpty()) {
            addMessage("Board atau game state belum tersedia.");
            return;
        }

        GameState state = stateOpt.orElseThrow();
        Board board = boardOpt.orElseThrow();
        String robberHexTileId = board.getRobber().getHexTileId();
        validTargets.addAll(robberValidator.getValidStealTargets(state, board, state.getCurrentPlayer().getId(), robberHexTileId)
                .stream()
                .filter(player -> player.getInventory().getTotalResourceCount() > 0)
                .toList());

        if (validTargets.isEmpty()) {
            addMessage("Tidak ada pemain valid untuk dicuri pada petak Nimon Ungu saat ini.");
            return;
        }

        for (AbstractPlayer player : validTargets) {
            targetBox.getChildren().add(createTargetRow(player));
        }
        selectPlayer(validTargets.get(0), (HBox) targetBox.getChildren().get(0));
    }

    private Optional<GameState> resolveGameState() {
        try {
            ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
            if (registry.contains(GameState.class)) {
                return Optional.of(registry.get(GameState.class));
            }
        } catch (IllegalStateException ignored) {
            // Scene Builder preview or context not initialized.
        }
        return Optional.empty();
    }

    private Optional<Board> resolveBoard() {
        try {
            ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
            if (registry.contains(Board.class)) {
                return Optional.of(registry.get(Board.class));
            }
        } catch (IllegalStateException ignored) {
            // Scene Builder preview or context not initialized.
        }
        return Optional.empty();
    }

    private HBox createTargetRow(AbstractPlayer player) {
        Label avatar = new Label(player.getName().substring(0, 1).toUpperCase());
        avatar.getStyleClass().add("player-avatar");
        Label info = new Label(player.getName() + " • Resources: " + player.getInventory().getTotalResourceCount());
        HBox row = new HBox(12, avatar, info);
        row.getStyleClass().add("steal-target-row");
        row.setOnMouseClicked(event -> selectPlayer(player, row));
        return row;
    }

    private void selectPlayer(AbstractPlayer player, HBox row) {
        if (selectedRow != null) {
            selectedRow.getStyleClass().remove("steal-target-row-selected");
        }
        selectedPlayer = player;
        selectedRow = row;
        if (!selectedRow.getStyleClass().contains("steal-target-row-selected")) {
            selectedRow.getStyleClass().add("steal-target-row-selected");
        }
    }

    private void addMessage(String message) {
        Label label = new Label(message);
        label.setWrapText(true);
        targetBox.getChildren().add(label);
    }

    @FXML private void handleCancel(ActionEvent event) { close(event); }

    @FXML
    private void handleConfirm(ActionEvent event) {
        if (validTargets.isEmpty()) {
            AlertUtil.info("Steal", "Tidak ada target yang bisa dicuri.");
            close(event);
            return;
        }
        if (selectedPlayer == null) {
            AlertUtil.warning("Steal", "Pilih pemain target terlebih dahulu.");
            return;
        }

        try {
            GameState state = resolveGameState().orElseThrow(() -> new IllegalStateException("Game state tidak tersedia"));
            RobberService robberService = resolveRobberService();
            var stolen = robberService.stealRandomResource(state, state.getCurrentPlayer().getId(), selectedPlayer.getId());
            if (stolen.isPresent()) {
                AlertUtil.info("Steal", state.getCurrentPlayer().getName() + " mencuri 1 " + display(stolen.get()) + " dari " + selectedPlayer.getName() + ".");
            } else {
                AlertUtil.info("Steal", selectedPlayer.getName() + " tidak memiliki resource untuk dicuri.");
            }
            close(event);
        } catch (RuntimeException exception) {
            AlertUtil.warning("Steal", exception.getMessage());
        }
    }

    private RobberService resolveRobberService() {
        ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(RobberService.class)) {
            return registry.get(RobberService.class);
        }

        RobberService service = new RobberService(ensureEventBus(registry));
        registry.register(RobberService.class, service);
        return service;
    }

    private GameEventBus ensureEventBus(ServiceRegistry registry) {
        if (!registry.contains(GameEventBus.class)) {
            registry.register(GameEventBus.class, new GameEventBus());
        }
        return registry.get(GameEventBus.class);
    }

    private String display(ResourceType type) {
        return switch (type) {
            case KAYU -> "WOOD";
            case BATU_BATA -> "BRICK";
            case GANDUM -> "WHEAT";
            case BIJIH -> "ORE";
            case PISANG -> "BANANA";
        };
    }

    private void close(ActionEvent event) { ((Stage)((Node)event.getSource()).getScene().getWindow()).close(); }
}
