package com.bananarepublic.ui.controller;

import com.bananarepublic.app.AppContext;
import com.bananarepublic.app.ServiceRegistry;
import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.card.CardType;
import com.bananarepublic.core.card.OwnedExperimentCard;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.service.CardService;
import com.bananarepublic.ui.util.AlertUtil;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.ChoiceDialog;
import javafx.scene.control.Label;
import javafx.scene.layout.FlowPane;
import javafx.scene.layout.Priority;
import javafx.scene.layout.Region;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public final class CardDialogController {
    @FXML private FlowPane cardsBox;
    @FXML private Label statusLabel;
    @FXML private Button playSelectedButton;

    private OwnedExperimentCard selectedCard;
    private VBox selectedCardNode;

    @FXML
    private void initialize() {
        renderOwnedCards();
    }

    private void renderOwnedCards() {
        selectedCard = null;
        selectedCardNode = null;
        cardsBox.getChildren().clear();
        GameState state = getState();
        List<OwnedExperimentCard> cards = state.getCurrentPlayer().getCardHand().getCards();
        if (cards.isEmpty()) {
            Label empty = new Label("Player aktif belum memiliki Kartu Temuan Dr. Neroifa.");
            empty.getStyleClass().add("cards-empty-state");
            cardsBox.getChildren().add(empty);
            setStatus("Tidak ada kartu yang dapat dipilih.");
            updatePlayButton();
            return;
        }
        cards.forEach(this::addOwnedCard);
        setStatus("Pilih kartu yang ingin dilihat atau dimainkan.");
        updatePlayButton();
    }

    private void addOwnedCard(OwnedExperimentCard ownedCard) {
        VBox card = new VBox(8);
        card.getStyleClass().addAll("experiment-card", styleClassFor(ownedCard.getType()));
        card.setPrefSize(172, 220);
        card.setMinSize(172, 220);
        card.setMaxSize(172, 220);
        card.setOnMouseClicked(event -> selectCard(ownedCard, card));
        Region spacer = new Region();
        VBox.setVgrow(spacer, Priority.ALWAYS);
        card.getChildren().addAll(
                label(playabilityFor(ownedCard.getType()), "card-chip"),
                titled(ownedCard.getCard().getCardName()),
                description(ownedCard.getCard().getDescription()),
                spacer,
                label("Bought turn " + ownedCard.getBoughtTurnNumber(), "card-meta")
        );
        cardsBox.getChildren().add(card);
    }

    private void selectCard(OwnedExperimentCard ownedCard, VBox node) {
        if (selectedCardNode != null) {
            selectedCardNode.getStyleClass().remove("experiment-card-selected");
        }
        selectedCard = ownedCard;
        selectedCardNode = node;
        selectedCardNode.getStyleClass().add("experiment-card-selected");
        setStatus(ownedCard.getCard().getCardName() + " dipilih. " + playabilityFor(ownedCard.getType()) + ".");
        updatePlayButton();
    }

    @FXML
    private void handleCancel(ActionEvent event) {
        close(event);
    }

    @FXML
    private void handleBuyCard() {
        try {
            OwnedExperimentCard boughtCard = getCardService().buyExperimentCard(getState());
            renderOwnedCards();
            setStatus("Membeli " + boughtCard.getCard().getCardName() + ".");
        } catch (RuntimeException ex) {
            AlertUtil.warning("Kartu Temuan", ex.getMessage());
        }
    }

    @FXML
    private void handleConfirm(ActionEvent event) {
        if (selectedCard == null) {
            AlertUtil.warning("Kartu Temuan", "Pilih kartu terlebih dahulu.");
            return;
        }
        if (selectedCard.getType() == CardType.VICTORY_POINT) {
            AlertUtil.info("Kartu Temuan", "Victory Point Card bersifat passive/secret dan tidak dimainkan manual.");
            return;
        }

        GameState state = getState();
        try {
            if (selectedCard.getType() == CardType.KNIGHT) {
                prepareKnightMove(event, state);
                return;
            }
            if (selectedCard.getType() == CardType.ROAD_BUILDING) {
                prepareRoadBuilding(event, state);
                return;
            }

            playSelectedCard(state);
            renderOwnedCards();
        } catch (RuntimeException ex) {
            AlertUtil.warning("Kartu Temuan", ex.getMessage());
        }
    }

    private void playSelectedCard(GameState state) {
        switch (selectedCard.getType()) {
            case KNIGHT -> throw new IllegalStateException("Nimon Ungu harus dipindahkan lewat map.");
            case ROAD_BUILDING -> throw new IllegalStateException("Road Building harus dipasang lewat map.");
            case MONOPOLY -> playMonopolyCard(state);
            case PLUGIN -> {
                getCardService().playPluginCard(state, selectedCard.getCardId());
                setStatus(selectedCard.getCard().getCardName() + " berhasil dimainkan.");
                return;
            }
            case VICTORY_POINT -> throw new IllegalStateException("Victory Point Card is passive");
        }
        setStatus(selectedCard.getCard().getCardName() + " berhasil dimainkan.");
    }

    private void prepareKnightMove(ActionEvent event, GameState state) {
        getCardService().validateCanPlayCard(state, selectedCard.getCardId(), CardType.KNIGHT);
        AppContext.getCurrent().serviceRegistry().register(
                PendingKnightCardSelection.class,
                new PendingKnightCardSelection(selectedCard.getCardId())
        );
        close(event);
    }

    private void prepareRoadBuilding(ActionEvent event, GameState state) {
        getCardService().validateCanPlayRoadBuildingCard(state, getBoard(), selectedCard.getCardId());
        AppContext.getCurrent().serviceRegistry().register(
                PendingRoadBuildingCardSelection.class,
                new PendingRoadBuildingCardSelection(selectedCard.getCardId())
        );
        close(event);
    }

    private void playMonopolyCard(GameState state) {
        ResourceType resourceType = promptResourceType("Pilih resource yang akan dimonopoli");
        if (resourceType == null) {
            throw new IllegalStateException("Resource monopoly belum dipilih.");
        }
        getCardService().playMonopolyCard(state, selectedCard.getCardId(), resourceType);
    }

    private ResourceType promptResourceType(String title) {
        Map<String, ResourceType> options = new LinkedHashMap<>();
        List<String> labels = new ArrayList<>();
        for (ResourceType type : ResourceType.values()) {
            String label = display(type);
            options.put(label, type);
            labels.add(label);
        }

        ChoiceDialog<String> dialog = new ChoiceDialog<>(labels.get(0), labels);
        dialog.setTitle(title);
        dialog.setHeaderText(title);
        return dialog.showAndWait().map(options::get).orElse(null);
    }

    private GameState getState() {
        return AppContext.getCurrent().serviceRegistry().get(GameState.class);
    }

    private Board getBoard() {
        ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
        if (!registry.contains(Board.class)) {
            throw new IllegalStateException("Board belum siap untuk kartu temuan.");
        }
        return registry.get(Board.class);
    }

    private CardService getCardService() {
        ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
        if (!registry.contains(CardService.class)) {
            registry.register(CardService.class, new CardService(ensureEventBus(registry)));
        }
        return registry.get(CardService.class);
    }

    private GameEventBus ensureEventBus(ServiceRegistry registry) {
        if (!registry.contains(GameEventBus.class)) {
            registry.register(GameEventBus.class, new GameEventBus());
        }
        return registry.get(GameEventBus.class);
    }

    private String playabilityFor(CardType type) {
        return switch (type) {
            case VICTORY_POINT -> "Passive / Secret";
            case PLUGIN -> "Playable Plugin";
            default -> "Playable";
        };
    }

    private String styleClassFor(CardType type) {
        return switch (type) {
            case KNIGHT -> "card-knight";
            case ROAD_BUILDING -> "card-road";
            case MONOPOLY -> "card-monopoly";
            case VICTORY_POINT -> "card-vp";
            case PLUGIN -> "card-plugin";
        };
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

    private Label label(String text, String styleClass) {
        Label label = new Label(text);
        label.getStyleClass().add(styleClass);
        label.setWrapText(true);
        return label;
    }

    private Label titled(String text) {
        return label(text, "card-title");
    }

    private Label description(String text) {
        return label(text, "card-desc");
    }

    private void setStatus(String message) {
        if (statusLabel != null) {
            statusLabel.setText(message);
        }
    }

    private void updatePlayButton() {
        if (playSelectedButton != null) {
            playSelectedButton.setDisable(selectedCard == null || selectedCard.getType() == CardType.VICTORY_POINT);
        }
    }

    private void close(ActionEvent event) {
        Stage s = (Stage) ((Node) event.getSource()).getScene().getWindow();
        s.close();
    }
}
