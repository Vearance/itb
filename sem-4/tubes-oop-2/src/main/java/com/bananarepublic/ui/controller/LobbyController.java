package com.bananarepublic.ui.controller;

import com.bananarepublic.app.AppContext;
import com.bananarepublic.app.ServiceRegistry;
import com.bananarepublic.core.bot.PlayerStrategy;
import com.bananarepublic.core.card.ExperimentCard;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerColor;
import com.bananarepublic.core.player.PlayerFactory;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.PluginLoadedEvent;
import com.bananarepublic.exception.plugin.PluginException;
import com.bananarepublic.plugin.PluginMetadata;
import com.bananarepublic.plugin.PluginRegistry;
import com.bananarepublic.plugin.bot.BotPlugin;
import com.bananarepublic.plugin.bot.BotPluginLoader;
import com.bananarepublic.ui.util.AlertUtil;
import com.bananarepublic.ui.viewmodel.GameViewModel;
import javafx.collections.FXCollections;
import javafx.fxml.FXML;
import javafx.scene.Node;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.control.ListCell;
import javafx.scene.control.TextField;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Pane;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import javafx.util.StringConverter;

import java.io.File;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

public final class LobbyController {
    @FXML private ComboBox<Integer> playerCountComboBox;
    @FXML private TextField playerOneField;
    @FXML private TextField playerTwoField;
    @FXML private TextField playerThreeField;
    @FXML private TextField playerFourField;
    @FXML private Node playerFourRow;
    @FXML private HBox colorOneRow;
    @FXML private HBox colorTwoRow;
    @FXML private HBox colorThreeRow;
    @FXML private HBox colorFourRow;
    @FXML private Label statusLabel;

    private final BotPluginLoader botPluginLoader = new BotPluginLoader();

    @FXML
    private void initialize() {
        playerCountComboBox.setItems(FXCollections.observableArrayList(3, 4));
        playerCountComboBox.setConverter(playerCountConverter());
        playerCountComboBox.setButtonCell(playerCountCell());
        playerCountComboBox.setCellFactory(listView -> playerCountCell());
        playerCountComboBox.setValue(3);
        playerCountComboBox.valueProperty().addListener((observable, oldValue, newValue) -> refreshPlayers());
        refreshPlayers();
    }

    @FXML
    private void handleBack() {
        AppContext.getCurrent().sceneManager().showMainMenu();
    }

    @FXML
    private void handleStartGame() {
        int playerCount = playerCountComboBox.getValue();
        List<String> names = new ArrayList<>();
        names.add(value(playerOneField, "Kebin"));
        names.add(value(playerTwoField, "Stewart"));
        names.add(value(playerThreeField, "Gro"));
        if (playerCount == 4) {
            names.add(value(playerFourField, "Toto"));
        }

        List<PlayerColor> colors = selectedColors(playerCount);
        if (colors.stream().distinct().count() != colors.size()) {
            AlertUtil.warning("Lobby", "Setiap pemain harus memakai warna yang unik.");
            return;
        }

        ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
        List<PlayerStrategy> botStrategies = createLoadedBotStrategies(registry);
        int botPlayerIndex = botStrategies.isEmpty() ? -1 : names.size() - 1;
        List<AbstractPlayer> players = new ArrayList<>();
        for (int i = 0; i < names.size(); i++) {
            if (i == botPlayerIndex) {
                PlayerStrategy strategy = botStrategies.getFirst();
                String botName = names.get(i).isBlank() ? strategy.getStrategyName() : names.get(i);
                players.add(PlayerFactory.createBotPlayer(i + 1, botName, colors.get(i), strategy));
            } else {
                players.add(PlayerFactory.createHumanPlayer(i + 1, names.get(i), colors.get(i)));
            }
        }
        GameState state = GameState.newGame(players);
        insertLoadedPluginCards(state, registry);
        registry.register(GameState.class, state);
        registry.register(GameViewModel.class, new GameViewModel());
        AppContext.getCurrent().sceneManager().showGame();
    }

    @FXML
    private void handleBotPlugin() {
        FileChooser chooser = new FileChooser();
        chooser.setTitle("Pilih Bot Plugin JAR");
        chooser.getExtensionFilters().add(new FileChooser.ExtensionFilter("JAR", "*.jar"));
        File file = chooser.showOpenDialog(stage());
        if (file == null) {
            return;
        }

        try {
            Path jarPath = file.toPath();
            List<BotPlugin> plugins = botPluginLoader.load(jarPath);
            ensurePluginRegistry().registerBotPlugins(plugins);
            publishLoaded(jarPath, plugins.stream().map(BotPlugin::getMetadata).toList());

            String loadedNames = plugins.stream()
                    .map(plugin -> plugin.getMetadata().name())
                    .reduce((left, right) -> left + ", " + right)
                    .orElse("Bot");
            statusLabel.setText("Bot plugin aktif: " + loadedNames + ". Slot pemain terakhir akan menjadi bot.");
            AlertUtil.info("Bot Plugin", plugins.size() + " bot plugin berhasil diload.");
        } catch (PluginException e) {
            statusLabel.setText("Bot plugin ditolak: " + e.getMessage());
            AlertUtil.error("Bot Plugin", e.getMessage());
        } catch (RuntimeException e) {
            statusLabel.setText("Gagal meload bot plugin: " + e.getMessage());
            AlertUtil.error("Bot Plugin", "Gagal meload bot plugin:\n" + e.getMessage());
        }
    }

    @FXML
    private void handleColorClick(MouseEvent event) {
        if (!(event.getSource() instanceof Circle selectedCircle) || !(selectedCircle.getParent() instanceof Pane colorRow)) {
            return;
        }

        for (Node node : colorRow.getChildren()) {
            if (node instanceof Circle circle) {
                circle.setStroke(Color.web("#cbbda7"));
                circle.setStrokeWidth(1);
            }
        }

        selectedCircle.setStroke(Color.web("#8b641e"));
        selectedCircle.setStrokeWidth(2);
        statusLabel.setText("Warna pemain diperbarui.");
    }

    private void refreshPlayers() {
        int playerCount = playerCountComboBox.getValue();
        playerFourRow.setVisible(playerCount == 4);
        playerFourRow.setManaged(playerCount == 4);
        if (ensurePluginRegistry().getBotPlugins().isEmpty()) {
            statusLabel.setText("Pilih jumlah pemain, nama pemain, dan warna unik.");
        } else {
            statusLabel.setText("Bot plugin aktif. Slot pemain terakhir akan menjadi bot.");
        }
    }

    private List<PlayerColor> selectedColors(int playerCount) {
        List<HBox> rows = new ArrayList<>(List.of(colorOneRow, colorTwoRow, colorThreeRow));
        if (playerCount == 4) {
            rows.add(colorFourRow);
        }
        return rows.stream().map(this::selectedColor).toList();
    }

    private void insertLoadedPluginCards(GameState state, ServiceRegistry registry) {
        if (!registry.contains(PluginRegistry.class)) {
            return;
        }

        List<ExperimentCard> pluginCards = registry.get(PluginRegistry.class).createCardInstances();
        state.getDevelopmentDeck().addCardsToTop(pluginCards);
    }

    private List<PlayerStrategy> createLoadedBotStrategies(ServiceRegistry registry) {
        if (!registry.contains(PluginRegistry.class)) {
            return List.of();
        }
        return registry.get(PluginRegistry.class).createBotStrategies();
    }

    private PluginRegistry ensurePluginRegistry() {
        ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
        if (!registry.contains(PluginRegistry.class)) {
            registry.register(PluginRegistry.class, new PluginRegistry());
        }
        return registry.get(PluginRegistry.class);
    }

    private void publishLoaded(Path jarPath, List<PluginMetadata> metadata) {
        ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
        if (!registry.contains(GameEventBus.class)) {
            registry.register(GameEventBus.class, new GameEventBus());
        }
        registry.get(GameEventBus.class).publish(new PluginLoadedEvent(jarPath.toAbsolutePath().normalize().toString(), metadata));
    }

    private Stage stage() {
        return statusLabel == null || statusLabel.getScene() == null
                ? null
                : (Stage) statusLabel.getScene().getWindow();
    }

    private PlayerColor selectedColor(HBox row) {
        for (int i = 0; i < row.getChildren().size(); i++) {
            Node node = row.getChildren().get(i);
            if (node instanceof Circle circle && circle.getStrokeWidth() >= 2) {
                return PlayerColor.values()[i];
            }
        }
        return PlayerColor.RED;
    }

    private String value(TextField field, String fallback) {
        return field == null || field.getText() == null || field.getText().isBlank()
                ? fallback
                : field.getText().trim();
    }

    private StringConverter<Integer> playerCountConverter() {
        return new StringConverter<>() {
            @Override
            public String toString(Integer value) {
                return value == null ? "" : value + " PLAYERS";
            }

            @Override
            public Integer fromString(String text) {
                if (text == null || text.isBlank()) {
                    return 3;
                }
                return Integer.parseInt(text.replace("PLAYERS", "").trim());
            }
        };
    }

    private ListCell<Integer> playerCountCell() {
        return new ListCell<>() {
            @Override
            protected void updateItem(Integer item, boolean empty) {
                super.updateItem(item, empty);
                setText(empty || item == null ? null : item + " PLAYERS");
            }
        };
    }
}
