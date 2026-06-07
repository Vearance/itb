package com.bananarepublic.ui.router;

import com.bananarepublic.ui.audio.SoundEffectPlayer;
import java.io.IOException;
import java.util.Objects;

import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.input.MouseButton;
import javafx.scene.input.MouseEvent;
import javafx.stage.Modality;
import javafx.stage.Stage;

public final class DialogManager {
    public void showInformation(String title, String message) {
        Alert alert = new Alert(Alert.AlertType.INFORMATION);
        alert.setTitle(title);
        alert.setHeaderText(null);
        alert.setContentText(message);
        alert.getDialogPane().addEventFilter(MouseEvent.MOUSE_PRESSED, event -> {
            if (event.getButton() == MouseButton.PRIMARY) {
                SoundEffectPlayer.getInstance().playMouseClick();
            }
        });
        alert.showAndWait();
    }

    public void showModal(String fxmlPath, String title) {
        try {
            FXMLLoader loader = new FXMLLoader(Objects.requireNonNull(getClass().getResource(fxmlPath), "FXML not found: " + fxmlPath));
            Parent root = loader.load();
            Stage stage = new Stage();
            stage.setTitle(title);
            stage.initModality(Modality.APPLICATION_MODAL);
            Scene scene = new Scene(root);
            scene.addEventFilter(MouseEvent.MOUSE_PRESSED, event -> {
                if (event.getButton() == MouseButton.PRIMARY) {
                    SoundEffectPlayer.getInstance().playMouseClick();
                }
            });
            scene.getStylesheets().add(Objects.requireNonNull(getClass().getResource("/css/main.css"), "Stylesheet not found: /css/main.css").toExternalForm());
            stage.setScene(scene);
            stage.showAndWait();
        } catch (IOException | NullPointerException exception) {
            showInformation(title, "Dialog belum dapat dibuka: " + exception.getMessage());
        }
    }
}
