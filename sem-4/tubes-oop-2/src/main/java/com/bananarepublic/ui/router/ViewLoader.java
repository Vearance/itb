package com.bananarepublic.ui.router;

import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;

import java.io.IOException;
import java.net.URL;

public final class ViewLoader {
    public Parent load(String fxmlPath) {
        URL resource = getClass().getResource(fxmlPath);
        if (resource == null) {
            throw new IllegalArgumentException("FXML resource not found: " + fxmlPath);
        }

        try {
            return FXMLLoader.load(resource);
        } catch (IOException exception) {
            throw new IllegalStateException("Failed to load FXML resource: " + fxmlPath, exception);
        }
    }
}
