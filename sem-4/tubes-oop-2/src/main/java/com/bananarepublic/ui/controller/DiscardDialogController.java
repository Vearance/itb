package com.bananarepublic.ui.controller;

import com.bananarepublic.app.AppContext;
import com.bananarepublic.core.action.DiscardResourceAction;
import com.bananarepublic.core.game.GameEngine;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.service.DiscardService;
import com.bananarepublic.ui.util.AlertUtil;
import com.bananarepublic.validator.RobberValidator;
import javafx.collections.FXCollections;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

import java.util.ArrayList;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

public final class DiscardDialogController {
    @FXML private ComboBox<String> playerComboBox;
    @FXML private Label counterLabel;
    @FXML private HBox resourceRows;

    private final Map<ResourceType, Integer> selected = new EnumMap<>(ResourceType.class);
    private final RobberValidator robberValidator = new RobberValidator();
    private final Map<String, AbstractPlayer> pendingPlayers = new java.util.LinkedHashMap<>();
    private AbstractPlayer targetPlayer;
    private int requiredDiscard;

    @FXML
    private void initialize() {
        GameState state = resolveGameState().orElse(null);
        initializePendingPlayers(state);
        initializePlayerCombo();
        selectInitialPlayer();
    }

    private void initializePendingPlayers(GameState state) {
        pendingPlayers.clear();
        if (state == null) {
            return;
        }
        for (AbstractPlayer player : state.getPlayers()) {
            if (state.hasPlayerDiscarded(player.getId())) {
                continue;
            }
            if (robberValidator.getRequiredDiscardCount(player) > 0) {
                pendingPlayers.put(playerLabel(player), player);
            }
        }
    }

    private void initializePlayerCombo() {
        if (playerComboBox == null) {
            return;
        }
        playerComboBox.setItems(FXCollections.observableArrayList(new ArrayList<>(pendingPlayers.keySet())));
        playerComboBox.valueProperty().addListener((observable, oldValue, newValue) -> loadPlayer(newValue));
    }

    private void selectInitialPlayer() {
        if (pendingPlayers.isEmpty()) {
            targetPlayer = null;
            requiredDiscard = 0;
            buildResourceRows();
            updateCounter();
            return;
        }

        String first = pendingPlayers.keySet().iterator().next();
        if (playerComboBox != null) {
            playerComboBox.getSelectionModel().select(first);
        }
        loadPlayer(first);
    }

    private void loadPlayer(String label) {
        targetPlayer = pendingPlayers.get(label);
        requiredDiscard = targetPlayer == null ? 0 : robberValidator.getRequiredDiscardCount(targetPlayer);
        buildResourceRows();
        updateCounter();
    }

    private void buildResourceRows() {
        resourceRows.getChildren().clear();
        for (ResourceType type : ResourceType.values()) {
            selected.put(type, 0);
            int owned = targetPlayer == null ? 0 : targetPlayer.getInventory().getResourceCount(type);
            resourceRows.getChildren().add(resourceSelector(type, owned));
        }
    }

    private Optional<GameState> resolveGameState() {
        try {
            if (AppContext.getCurrent().serviceRegistry().contains(GameState.class)) {
                return Optional.of(AppContext.getCurrent().serviceRegistry().get(GameState.class));
            }
        } catch (IllegalStateException ignored) {
            // Dialog can still be previewed in Scene Builder without an active game state.
        }
        return Optional.empty();
    }

    private VBox resourceSelector(ResourceType type, int owned) {
        Label title = new Label(display(type));
        title.getStyleClass().add("resource-name");

        Label ownedLabel = new Label("Owned: " + owned);

        Label amount = new Label("0");
        amount.getStyleClass().add("resource-amount");

        Button minus = new Button("-");
        Button plus = new Button("+");
        minus.setOnAction(e -> change(type, -1, owned, amount));
        plus.setOnAction(e -> change(type, 1, owned, amount));

        HBox controls = new HBox(6, minus, amount, plus);
        controls.setAlignment(Pos.CENTER);

        VBox box = new VBox(8, title, ownedLabel, controls);
        box.getStyleClass().add("discard-resource-box");
        box.setAlignment(Pos.CENTER);
        return box;
    }

    private void change(ResourceType type, int delta, int owned, Label amount) {
        int currentForType = selected.get(type);
        int currentTotal = selected.values().stream().mapToInt(Integer::intValue).sum();
        if (delta > 0 && currentTotal >= requiredDiscard) {
            return;
        }
        int next = Math.max(0, Math.min(owned, currentForType + delta));
        selected.put(type, next);
        amount.setText(String.valueOf(next));
        updateCounter();
    }

    private void updateCounter() {
        int selectedCount = selected.values().stream().mapToInt(Integer::intValue).sum();
        counterLabel.setText(selectedCount + "/" + requiredDiscard + " cards selected");
    }

    private ResourceBundle selectedBundle() {
        ResourceBundle bundle = ResourceBundle.empty();
        selected.forEach(bundle::set);
        return bundle;
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

    private String playerLabel(AbstractPlayer player) {
        return player.getName() + " (" + player.getId().getValue() + ", " + player.getInventory().getTotalResourceCount() + " cards)";
    }

    @FXML
    private void handleCancel(ActionEvent event) {
        if (requiredDiscard > 0) {
            AlertUtil.warning("Discard", "Discard wajib diselesaikan sebelum Nimon Ungu dipindahkan.");
            return;
        }
        close(event);
    }

    @FXML
    private void handleConfirm(ActionEvent event) {
        int selectedCount = selected.values().stream().mapToInt(Integer::intValue).sum();
        if (selectedCount != requiredDiscard) {
            AlertUtil.warning("Discard", "Pilih tepat " + requiredDiscard + " kartu untuk dibuang.");
            return;
        }
        if (targetPlayer == null) {
            AlertUtil.warning("Discard", "Tidak ada pemain yang perlu discard.");
            close(event);
            return;
        }

        try {
            GameState state = AppContext.getCurrent().serviceRegistry().get(GameState.class);
            DiscardResourceAction action = new DiscardResourceAction(targetPlayer.getId(), selectedBundle());
            if (AppContext.getCurrent().serviceRegistry().contains(GameEngine.class)) {
                AppContext.getCurrent().serviceRegistry().get(GameEngine.class).discardResources(action);
            } else if (AppContext.getCurrent().serviceRegistry().contains(DiscardService.class)) {
                DiscardService service = AppContext.getCurrent().serviceRegistry().get(DiscardService.class);
                service.discardResources(state, action);
            } else {
                targetPlayer.getInventory().spendResourceBundle(selectedBundle());
                state.getBank().receiveResourceBundle(selectedBundle());
            }
            close(event);
        } catch (RuntimeException exception) {
            AlertUtil.warning("Discard", exception.getMessage());
        }
    }

    private void close(ActionEvent event) {
        ((Stage) ((Node) event.getSource()).getScene().getWindow()).close();
    }
}
