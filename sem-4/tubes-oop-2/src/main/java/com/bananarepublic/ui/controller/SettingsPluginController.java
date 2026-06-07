package com.bananarepublic.ui.controller;

import com.bananarepublic.app.AppContext;
import com.bananarepublic.app.ServiceRegistry;
import com.bananarepublic.core.card.ExperimentCard;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.PluginLoadedEvent;
import com.bananarepublic.exception.plugin.PluginException;
import com.bananarepublic.plugin.PluginMetadata;
import com.bananarepublic.plugin.PluginRegistry;
import com.bananarepublic.plugin.card.ExperimentCardPlugin;
import com.bananarepublic.plugin.card.ExperimentCardPluginLoader;
import com.bananarepublic.service.PersistenceService;
import com.bananarepublic.ui.audio.BackgroundMusicPlayer;
import com.bananarepublic.ui.util.AlertUtil;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.Label;
import javafx.scene.control.ListView;
import javafx.scene.control.Slider;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import javafx.scene.Node;

import java.io.File;
import java.nio.file.Path;
import java.util.List;

public final class SettingsPluginController {
    @FXML private ListView<String> pluginListView;
    @FXML private Label pluginStatusLabel;
    @FXML private Slider musicVolumeSlider;
    @FXML private Label musicVolumeValueLabel;

    private final ExperimentCardPluginLoader pluginLoader = new ExperimentCardPluginLoader();

    @FXML
    private void initialize() {
        configureMusicVolumeSlider();
        renderLoadedPlugins();
    }

    private void configureMusicVolumeSlider() {
        if (musicVolumeSlider == null) {
            return;
        }

        BackgroundMusicPlayer player = BackgroundMusicPlayer.getInstance();
        musicVolumeSlider.setMin(0);
        musicVolumeSlider.setMax(100);
        musicVolumeSlider.setValue(player.getVolume() * 100.0);
        updateMusicVolumeLabel(player.getVolume());

        musicVolumeSlider.valueProperty().addListener((observable, oldValue, newValue) -> {
            double volume = newValue.doubleValue() / 100.0;
            player.setVolume(volume);
            updateMusicVolumeLabel(volume);
        });
    }

    private void updateMusicVolumeLabel(double volume) {
        if (musicVolumeValueLabel != null) {
            musicVolumeValueLabel.setText(Math.round(volume * 100.0) + "%");
        }
    }

    @FXML
    private void handleClose(ActionEvent event) {
        close(event);
    }

    @FXML
    private void handleCancel(ActionEvent event) {
        close(event);
    }

    @FXML
    private void handleConfirm(ActionEvent event) {
        close(event);
    }

    @FXML
    private void handleSave() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (!registry.contains(com.bananarepublic.core.game.GameState.class)
                || !registry.contains(com.bananarepublic.core.board.Board.class)) {
            AlertUtil.warning("Save Game", "Tidak ada permainan yang sedang berlangsung untuk disimpan.");
            return;
        }

        var state = registry.get(com.bananarepublic.core.game.GameState.class);
        var board = registry.get(com.bananarepublic.core.board.Board.class);

        FileChooser chooser = new FileChooser();
        chooser.setTitle("Simpan Permainan");
        chooser.getExtensionFilters().add(new FileChooser.ExtensionFilter(
                "Banana Republic Save", "*.banana-republic.json"
        ));
        chooser.setInitialFileName(state.getCurrentPlayer().getName() + "-turn" + state.getTurnNumber());

        File file = chooser.showSaveDialog(null);
        if (file == null) {
            return;
        }

        try {
            new PersistenceService().save(state, board, file.toPath());
            AlertUtil.info("Save Game", "Permainan berhasil disimpan ke:\n" + file.getAbsolutePath());
        } catch (Exception e) {
            AlertUtil.error("Save Game", "Gagal menyimpan permainan:\n" + e.getMessage());
        }
    }

    @FXML
    private void handleLoadPlugin(ActionEvent event) {
        FileChooser chooser = new FileChooser();
        chooser.setTitle("Pilih Plugin JAR");
        chooser.getExtensionFilters().add(new FileChooser.ExtensionFilter("JAR", "*.jar"));
        File file = chooser.showOpenDialog(stage(event));
        if (file == null) {
            return;
        }

        try {
            Path jarPath = file.toPath();
            List<ExperimentCardPlugin> plugins = pluginLoader.load(jarPath);
            ensurePluginRegistry().registerCardPlugins(plugins);
            int insertedCards = insertPluginsIntoActiveDeck(plugins);
            publishLoaded(jarPath, plugins);
            renderLoadedPlugins();

            String message = plugins.size() + " plugin kartu berhasil diload.";
            if (insertedCards > 0) {
                message += "\n" + insertedCards + " kartu plugin masuk ke deck dan siap dibeli.";
            } else {
                message += "\nBelum ada game aktif, kartu akan tersedia setelah game dimulai.";
            }
            AlertUtil.info("Plugin", message);
        } catch (PluginException e) {
            setStatus("Plugin ditolak: " + e.getMessage());
            AlertUtil.error("Plugin", e.getMessage());
        } catch (RuntimeException e) {
            setStatus("Gagal meload plugin: " + e.getMessage());
            AlertUtil.error("Plugin", "Gagal meload plugin:\n" + e.getMessage());
        }
    }

    @FXML
    private void handleMainMenu() {
        AppContext.getCurrent().sceneManager().showMainMenu();
    }

    @FXML
    private void handleViewBoard() {
        AppContext.getCurrent().sceneManager().showGame();
    }

    private void close(ActionEvent event) {
        Stage s = stage(event);
        if (s != null) {
            s.close();
        }
    }

    private Stage stage(ActionEvent event) {
        return event == null ? null : (Stage) ((Node) event.getSource()).getScene().getWindow();
    }

    private int insertPluginsIntoActiveDeck(List<ExperimentCardPlugin> plugins) {
        ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
        if (!registry.contains(GameState.class)) {
            return 0;
        }

        GameState state = registry.get(GameState.class);
        List<ExperimentCard> cards = plugins.stream()
                .map(ExperimentCardPlugin::createCard)
                .toList();
        state.getDevelopmentDeck().addCardsToTop(cards);
        return cards.size();
    }

    private void publishLoaded(Path jarPath, List<ExperimentCardPlugin> plugins) {
        List<PluginMetadata> metadata = plugins.stream()
                .map(ExperimentCardPlugin::getMetadata)
                .toList();
        ensureEventBus().publish(new PluginLoadedEvent(jarPath.toAbsolutePath().normalize().toString(), metadata));
    }

    private void renderLoadedPlugins() {
        if (pluginListView == null) {
            return;
        }
        List<String> items = ensurePluginRegistry().getLoadedMetadata().stream()
                .map(metadata -> metadata.name() + " (" + metadata.className() + ")")
                .toList();
        pluginListView.getItems().setAll(items);
        setStatus(items.isEmpty()
                ? "Belum ada plugin kartu yang diload."
                : items.size() + " plugin kartu aktif.");
    }

    private PluginRegistry ensurePluginRegistry() {
        ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
        if (!registry.contains(PluginRegistry.class)) {
            registry.register(PluginRegistry.class, new PluginRegistry());
        }
        return registry.get(PluginRegistry.class);
    }

    private GameEventBus ensureEventBus() {
        ServiceRegistry registry = AppContext.getCurrent().serviceRegistry();
        if (!registry.contains(GameEventBus.class)) {
            registry.register(GameEventBus.class, new GameEventBus());
        }
        return registry.get(GameEventBus.class);
    }

    private void setStatus(String message) {
        if (pluginStatusLabel != null) {
            pluginStatusLabel.setText(message);
        }
    }
}
