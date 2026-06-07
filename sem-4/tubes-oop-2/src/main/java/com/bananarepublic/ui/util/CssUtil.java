package com.bananarepublic.ui.util;

import javafx.scene.Scene;
import java.net.URL;

public final class CssUtil {
    private CssUtil() {}
    public static void attach(Scene scene, String cssPath) {
        URL url = CssUtil.class.getResource(cssPath);
        if (scene != null && url != null) scene.getStylesheets().add(url.toExternalForm());
    }
}
