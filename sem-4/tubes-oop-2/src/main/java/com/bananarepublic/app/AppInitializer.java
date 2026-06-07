package com.bananarepublic.app;

import java.util.Objects;

import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.plugin.PluginRegistry;
import com.bananarepublic.service.CardService;
import com.bananarepublic.timer.TurnTimer;
import com.bananarepublic.ui.router.DialogManager;
import com.bananarepublic.ui.router.SceneManager;
import com.bananarepublic.ui.router.ViewLoader;

import javafx.stage.Stage;

/*
    Create global services and open first screen
*/
public final class AppInitializer {
    public AppContext initialize(Stage primaryStage) {
        Objects.requireNonNull(primaryStage, "primaryStage");
        ServiceRegistry serviceRegistry = new ServiceRegistry();
        GameEventBus eventBus = new GameEventBus();
        serviceRegistry.register(GameEventBus.class, eventBus);
        serviceRegistry.register(PluginRegistry.class, new PluginRegistry());
        serviceRegistry.register(CardService.class, new CardService(eventBus));
        serviceRegistry.register(TurnTimer.class, new TurnTimer(eventBus));

        ViewLoader viewLoader = new ViewLoader();
        DialogManager dialogManager = new DialogManager();
        SceneManager sceneManager = new SceneManager(primaryStage, viewLoader, dialogManager);
        AppContext context = new AppContext(serviceRegistry, sceneManager);

        AppContext.setCurrent(context);
        sceneManager.showMainMenu();
        return context;
    }
}
