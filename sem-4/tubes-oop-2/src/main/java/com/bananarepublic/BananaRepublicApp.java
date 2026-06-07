package com.bananarepublic;

import com.bananarepublic.app.AppInitializer;
import javafx.application.Application;
import javafx.stage.Stage;

/* 
    JavaFX entrypoint
*/
public class BananaRepublicApp extends Application {
    @Override
    public void start(Stage primaryStage) {
        new AppInitializer().initialize(primaryStage);
    }

    public static void main(String[] args) {
        launch(args);
    }
}
