package com.bananarepublic.app;

import java.util.Objects;

import com.bananarepublic.ui.router.SceneManager;

/*
    Access to services(?)
*/
public final class AppContext {
    private static AppContext current; // singleton
    private final ServiceRegistry serviceRegistry;
    private final SceneManager sceneManager;

    public AppContext(ServiceRegistry serviceRegistry, SceneManager sceneManager) {
        this.serviceRegistry = Objects.requireNonNull(serviceRegistry, "serviceRegistry");
        this.sceneManager = Objects.requireNonNull(sceneManager, "sceneManager");
    }

    public static void setCurrent(AppContext context) {
        current = Objects.requireNonNull(context, "context");
    }

    public static AppContext getCurrent() {
        if (current == null) {
            throw new IllegalStateException("AppContext has not been initialized");
        }
        return current;
    }

    public ServiceRegistry serviceRegistry() {
        return serviceRegistry;
    }

    public SceneManager sceneManager() {
        return sceneManager;
    }
}
