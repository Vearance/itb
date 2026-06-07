package com.bananarepublic.plugin;

import java.nio.file.Path;
import java.util.List;

public interface PluginLoader<T extends Plugin> {
    List<T> load(Path jarPath);
}
