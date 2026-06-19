package com.bananarepublic;

/**
 * Launcher class that bootstraps the JavaFX application.
 *
 * This indirection is needed because when JavaFX classes are bundled in
 * a fat JAR (via maven-shade-plugin), extending {@link javafx.application.Application}
 * directly from the entry point causes JavaFX to detect itself in the unnamed
 * module and refuse to start with "JavaFX runtime components are missing".
 *
 * By keeping the main class separate (not extending Application), the
 * {@link Application#launch(Class, String[])} method works correctly from
 * the classpath.
 */
public final class Launcher {
    public static void main(String[] args) {
        BananaRepublicApp.main(args);
    }

    private Launcher() {}
}
