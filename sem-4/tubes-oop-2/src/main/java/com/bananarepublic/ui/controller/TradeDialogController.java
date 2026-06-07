package com.bananarepublic.ui.controller;

import com.bananarepublic.app.AppContext;
import com.bananarepublic.app.ServiceRegistry;
import com.bananarepublic.core.action.TradeAction;
import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.exception.trade.InvalidTradeException;
import com.bananarepublic.service.TradeService;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.geometry.Pos;
import javafx.scene.Node;
import com.bananarepublic.ui.component.ResourceIconFactory;
import javafx.scene.control.Button;
import javafx.scene.control.ButtonType;
import javafx.scene.control.ChoiceDialog;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Dialog;
import javafx.scene.control.Label;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

import java.util.ArrayList;
import java.util.EnumMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

public final class TradeDialogController {
    private static final String BANK_TARGET = "Bank / Pelabuhan";

    @FXML private HBox giveBox;
    @FXML private HBox receiveBox;
    @FXML private ComboBox<String> targetCombo;
    @FXML private Label validationLabel;

    private final Map<ResourceType, Integer> give = new EnumMap<>(ResourceType.class);
    private final Map<ResourceType, Integer> receive = new EnumMap<>(ResourceType.class);
    private final Map<String, PlayerId> targetIds = new LinkedHashMap<>();

    @FXML
    private void initialize() {
        initializeResourceMaps();
        buildResourceSelectors();
        refreshTargets();
        showHint("Bank trade memakai rasio terbaik: 4:1 default, 3:1 pelabuhan umum, atau 2:1 pelabuhan khusus.");
    }

    @FXML
    private void handleCancel(ActionEvent event) {
        close(event);
    }

    @FXML
    private void handleConfirm(ActionEvent event) {
        clearValidationError();
        try {
            executeSelectedTrade();
            close(event);
        } catch (RuntimeException exception) {
            showValidationError(exception.getMessage());
        }
    }

    private void initializeResourceMaps() {
        for (ResourceType type : ResourceType.values()) {
            give.put(type, 0);
            receive.put(type, 0);
        }
    }

    private void buildResourceSelectors() {
        giveBox.getChildren().clear();
        receiveBox.getChildren().clear();
        for (ResourceType type : ResourceType.values()) {
            giveBox.getChildren().add(selector(type, give, owned(type)));
            receiveBox.getChildren().add(selector(type, receive, 19));
        }
    }

    private VBox selector(ResourceType type, Map<ResourceType, Integer> resources, int max) {
        Node icon = ResourceIconFactory.iconForResource(type, 42);
        icon.getStyleClass().add("trade-resource-icon");


        Label amount = new Label(String.valueOf(resources.get(type)));
        amount.getStyleClass().add("resource-amount");

        Button minus = new Button("-");
        Button plus = new Button("+");
        minus.setOnAction(event -> change(type, resources, -1, max, amount));
        plus.setOnAction(event -> change(type, resources, 1, max, amount));

        HBox controls = new HBox(6, minus, amount, plus);
        controls.setAlignment(Pos.CENTER);

        VBox box = new VBox(8, icon, controls);
        box.getStyleClass().add("trade-resource-box");
        box.setAlignment(Pos.CENTER);
        return box;
    }

    private void change(ResourceType type, Map<ResourceType, Integer> resources, int delta, int max, Label amount) {
        int next = Math.max(0, Math.min(max, resources.get(type) + delta));
        resources.put(type, next);
        amount.setText(String.valueOf(next));
    }

    private void refreshTargets() {
        targetCombo.getItems().clear();
        targetIds.clear();
        targetCombo.getItems().add(BANK_TARGET);

        GameState state = getState();
        state.getAllPlayers().stream()
                .filter(player -> !player.getId().equals(state.getCurrentPlayer().getId()))
                .forEach(player -> {
                    String label = targetLabel(player);
                    targetIds.put(label, player.getId());
                    targetCombo.getItems().add(label);
                });

        targetCombo.getSelectionModel().selectFirst();
    }

    private void executeSelectedTrade() {
        GameState state = getState();
        TradeService tradeService = getTradeService();
        ResourceBundle offeredResources = new ResourceBundle(give);
        ResourceBundle requestedResources = new ResourceBundle(receive);
        String selectedTarget = targetCombo.getValue();

        if (BANK_TARGET.equals(selectedTarget)) {
            executeBankTrade(state, tradeService, offeredResources, requestedResources);
            return;
        }

        executePlayerTrade(state, tradeService, selectedTarget, offeredResources, requestedResources);
    }

    private void executeBankTrade(
            GameState state,
            TradeService tradeService,
            ResourceBundle offeredResources,
            ResourceBundle requestedResources
    ) {
        ResourceType offeredType = singleSelectedType(
                offeredResources,
                "Pilih satu resource yang ingin diberikan ke bank.",
                "Bank trade hanya bisa memberikan satu jenis resource."
        );
        ResourceType requestedType = singleSelectedType(
                requestedResources,
                "Pilih satu resource yang ingin diterima dari bank.",
                "Bank trade hanya bisa menerima satu jenis resource."
        );

        if (offeredType == requestedType) {
            throw new InvalidTradeException("Cannot trade a resource for the same resource");
        }

        Board board = getBoard();
        int ratio = tradeService.getBestHarborTradeRatio(board, state.getCurrentPlayer().getId(), offeredType);
        if (offeredResources.get(offeredType) != ratio) {
            throw new InvalidTradeException("Bank trade " + display(offeredType) + " membutuhkan " + ratio + " kartu.");
        }
        if (requestedResources.get(requestedType) != 1) {
            throw new InvalidTradeException("Bank trade hanya menerima 1 kartu resource.");
        }

        tradeService.tradeWithHarbor(state, board, offeredType, requestedType);
    }

    private void executePlayerTrade(
            GameState state,
            TradeService tradeService,
            String selectedTarget,
            ResourceBundle offeredResources,
            ResourceBundle requestedResources
    ) {
        PlayerId targetId = targetIds.get(selectedTarget);
        if (targetId == null) {
            throw new InvalidTradeException("Pilih pemain target trade.");
        }

        TradeAction action = new TradeAction(
                state.getCurrentPlayer().getId(),
                targetId,
                offeredResources,
                requestedResources
        );
        TradeAction pendingTrade = tradeService.requestNormalTrade(state, action);
        resolveTradeNegotiation(state, tradeService, pendingTrade);
    }

    private void resolveTradeNegotiation(GameState state, TradeService tradeService, TradeAction pendingTrade) {
        TradeAction currentTrade = pendingTrade;
        while (true) {
            String response = promptTradeResponse(currentTrade);
            PlayerId responderId = currentTrade.getTargetId();
            if ("Reject".equals(response) || response == null) {
                tradeService.rejectNormalTrade(currentTrade, responderId, "Trade rejected");
                return;
            }
            if ("Accept".equals(response)) {
                tradeService.acceptNormalTrade(state, currentTrade, responderId);
                return;
            }

            CounterOffer counterOffer = promptCounterOffer(state, responderId, currentTrade.getInitiatorId());
            if (counterOffer == null) {
                tradeService.rejectNormalTrade(currentTrade, responderId, "Counter-offer canceled");
                return;
            }
            currentTrade = tradeService.counterOfferNormalTrade(state, currentTrade, counterOffer.offered(), counterOffer.requested());
        }
    }

    private String promptTradeResponse(TradeAction tradeAction) {
        GameState state = getState();
        String initiatorName = playerName(state, tradeAction.getInitiatorId());
        String targetName = playerName(state, tradeAction.getTargetId());
        List<String> options = List.of("Accept", "Reject", "Counter Offer");
        ChoiceDialog<String> dialog = new ChoiceDialog<>(options.get(0), options);
        dialog.setTitle("Trade Response");
        dialog.setHeaderText(targetName + ", respon trade dari " + initiatorName);
        dialog.setContentText(initiatorName + " memberi " + bundleText(tradeAction.getOfferedResources())
                + "\n" + initiatorName + " meminta " + bundleText(tradeAction.getRequestedResources()));
        Optional<String> result = dialog.showAndWait();
        return result.orElse("Reject");
    }

    private CounterOffer promptCounterOffer(GameState state, PlayerId offererId, PlayerId recipientId) {
        String offererName = playerName(state, offererId);
        String recipientName = playerName(state, recipientId);
        Map<ResourceType, Integer> offered = emptyResourceSelection();
        Map<ResourceType, Integer> requested = emptyResourceSelection();
        HBox offeredBox = new HBox(10);
        HBox requestedBox = new HBox(10);
        AbstractPlayer offerer = state.getPlayerById(offererId);
        AbstractPlayer recipient = state.getPlayerById(recipientId);
        for (ResourceType type : ResourceType.values()) {
            offeredBox.getChildren().add(selector(type, offered, offerer.getResourceCount(type)));
            requestedBox.getChildren().add(selector(type, requested, recipient.getResourceCount(type)));
        }

        Label error = new Label("");
        error.getStyleClass().add("trade-error");
        Dialog<CounterOffer> dialog = new Dialog<>();
        dialog.setTitle("Counter Offer");
        dialog.setHeaderText(offererName + " membuat counter-offer untuk " + recipientName);
        dialog.getDialogPane().getButtonTypes().addAll(ButtonType.CANCEL, ButtonType.OK);
        VBox content = new VBox(
                12,
                new Label(offererName + " gives"),
                offeredBox,
                new Label(offererName + " receives"),
                requestedBox,
                error
        );
        dialog.getDialogPane().setContent(content);
        dialog.setResultConverter(button -> button == ButtonType.OK
                ? new CounterOffer(new ResourceBundle(offered), new ResourceBundle(requested))
                : null);
        Node okButton = dialog.getDialogPane().lookupButton(ButtonType.OK);
        okButton.addEventFilter(ActionEvent.ACTION, event -> {
            try {
                validateCounterOfferSelection(new ResourceBundle(offered), new ResourceBundle(requested));
            } catch (InvalidTradeException exception) {
                error.setText(exception.getMessage());
                event.consume();
            }
        });
        return dialog.showAndWait().orElse(null);
    }

    private record CounterOffer(ResourceBundle offered, ResourceBundle requested) {}

    private Map<ResourceType, Integer> emptyResourceSelection() {
        Map<ResourceType, Integer> resources = new EnumMap<>(ResourceType.class);
        for (ResourceType type : ResourceType.values()) {
            resources.put(type, 0);
        }
        return resources;
    }

    private void validateCounterOfferSelection(ResourceBundle offered, ResourceBundle requested) {
        if (offered.total() == 0 || requested.total() == 0) {
            throw new InvalidTradeException("Counter-offer cannot give or request resources for free");
        }
        for (ResourceType type : ResourceType.values()) {
            if (offered.get(type) > 0 && requested.get(type) > 0) {
                throw new InvalidTradeException("Cannot trade the same resource type both ways: " + display(type));
            }
        }
    }

    private ResourceType singleSelectedType(ResourceBundle resources, String noneMessage, String multipleMessage) {
        ResourceType selectedType = null;
        for (ResourceType type : ResourceType.values()) {
            if (resources.get(type) == 0) {
                continue;
            }
            if (selectedType != null) {
                throw new InvalidTradeException(multipleMessage);
            }
            selectedType = type;
        }
        if (selectedType == null) {
            throw new InvalidTradeException(noneMessage);
        }
        return selectedType;
    }

    private int owned(ResourceType type) {
        return getState().getCurrentPlayer().getInventory().getResourceCount(type);
    }

    private String targetLabel(AbstractPlayer player) {
        return player.getName() + " (" + player.getId().getValue() + ")";
    }

    private String playerName(GameState state, PlayerId playerId) {
        return state.findPlayerById(playerId)
                .map(AbstractPlayer::getName)
                .orElse(playerId.getValue());
    }

    private String bundleText(ResourceBundle bundle) {
        List<String> parts = new ArrayList<>();
        for (ResourceType type : ResourceType.values()) {
            int amount = bundle.get(type);
            if (amount > 0) {
                parts.add(amount + " " + display(type));
            }
        }
        return parts.isEmpty() ? "0 resource" : String.join(", ", parts);
    }

    private String display(ResourceType type) {
        return ResourceIconFactory.display(type);
    }

    private GameState getState() {
        ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
        if (!registry.contains(GameState.class)) {
            throw new InvalidTradeException("Game belum siap untuk trade.");
        }
        return registry.get(GameState.class);
    }

    private Board getBoard() {
        ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
        if (!registry.contains(Board.class)) {
            throw new InvalidTradeException("Board belum siap untuk trade.");
        }
        return registry.get(Board.class);
    }

    private TradeService getTradeService() {
        ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(TradeService.class)) {
            return registry.get(TradeService.class);
        }

        TradeService tradeService = new TradeService(getEventBus());
        registry.register(TradeService.class, tradeService);
        return tradeService;
    }

    private GameEventBus getEventBus() {
        ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(GameEventBus.class)) {
            return registry.get(GameEventBus.class);
        }

        GameEventBus eventBus = new GameEventBus();
        registry.register(GameEventBus.class, eventBus);
        return eventBus;
    }

    private void clearValidationError() {
        validationLabel.getStyleClass().remove("trade-error");
        showHint("Trade akan langsung dijalankan jika valid.");
    }

    private void showHint(String message) {
        validationLabel.setText(message);
    }

    private void showValidationError(String message) {
        validationLabel.setText(message == null || message.isBlank() ? "Trade tidak valid." : message);
        if (!validationLabel.getStyleClass().contains("trade-error")) {
            validationLabel.getStyleClass().add("trade-error");
        }
    }

    private void close(ActionEvent event) {
        ((Stage) ((Node) event.getSource()).getScene().getWindow()).close();
    }
}
