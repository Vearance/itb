package com.bananarepublic.ui.util;

import javafx.application.Platform;

public final class FxThreadUtil {
    private FxThreadUtil() {}
    public static void runOnFxThread(Runnable task) {
        if (task == null) return;
        if (Platform.isFxApplicationThread()) task.run(); else Platform.runLater(task);
    }
}
