package com.bananarepublic.ui.component;

import javafx.scene.image.Image;

import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;

final class ResourceImageCache {
    private static final Map<String, Image> IMAGES = new ConcurrentHashMap<>();

    private ResourceImageCache() {
    }

    static Image get(String resourcePath) {
        return IMAGES.computeIfAbsent(resourcePath, ResourceImageCache::load);
    }

    private static Image load(String resourcePath) {
        return new Image(Objects.requireNonNull(
                ResourceImageCache.class.getResource(resourcePath),
                "Image asset not found: " + resourcePath
        ).toExternalForm());
    }
}
