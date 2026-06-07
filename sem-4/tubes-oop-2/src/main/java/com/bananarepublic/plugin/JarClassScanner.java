package com.bananarepublic.plugin;

import com.bananarepublic.exception.plugin.PluginLoadException;

import java.io.IOException;
import java.nio.file.Path;
import java.util.List;
import java.util.jar.JarFile;

public final class JarClassScanner {
    public List<String> scan(Path jarPath) {
        try (JarFile jarFile = new JarFile(jarPath.toFile())) {
            return jarFile.stream()
                    .filter(entry -> !entry.isDirectory())
                    .map(entry -> entry.getName())
                    .filter(name -> name.endsWith(".class"))
                    .filter(name -> !name.endsWith("module-info.class"))
                    .filter(name -> !name.endsWith("package-info.class"))
                    .map(this::toClassName)
                    .sorted()
                    .toList();
        } catch (IOException e) {
            throw new PluginLoadException("Unable to read plugin JAR: " + jarPath, e);
        }
    }

    private String toClassName(String entryName) {
        return entryName
                .substring(0, entryName.length() - ".class".length())
                .replace('/', '.')
                .replace('\\', '.');
    }
}
