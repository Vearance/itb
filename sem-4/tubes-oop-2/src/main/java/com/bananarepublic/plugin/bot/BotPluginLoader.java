package com.bananarepublic.plugin.bot;

import com.bananarepublic.core.bot.PlayerStrategy;
import com.bananarepublic.exception.plugin.PluginLoadException;
import com.bananarepublic.exception.plugin.PluginValidationException;
import com.bananarepublic.plugin.JarClassScanner;
import com.bananarepublic.plugin.PluginLoader;
import com.bananarepublic.plugin.PluginMetadata;
import com.bananarepublic.plugin.PluginValidator;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.nio.file.Path;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public final class BotPluginLoader implements PluginLoader<BotPlugin> {
    private final JarClassScanner classScanner;
    private final PluginValidator validator;

    public BotPluginLoader() {
        this(new JarClassScanner(), new PluginValidator());
    }

    public BotPluginLoader(JarClassScanner classScanner, PluginValidator validator) {
        this.classScanner = Objects.requireNonNull(classScanner, "classScanner");
        this.validator = Objects.requireNonNull(validator, "validator");
    }

    @Override
    public List<BotPlugin> load(Path jarPath) {
        validator.validateJarPath(jarPath);
        URLClassLoader classLoader = createClassLoader(jarPath);
        List<BotPlugin> plugins = new ArrayList<>();

        for (String className : classScanner.scan(jarPath)) {
            Class<?> loadedClass = loadClass(classLoader, className);
            if (!PlayerStrategy.class.isAssignableFrom(loadedClass)) {
                continue;
            }

            validator.validatePlayerStrategyClass(loadedClass);
            Class<? extends PlayerStrategy> strategyClass = loadedClass.asSubclass(PlayerStrategy.class);
            PlayerStrategy prototype = instantiate(strategyClass);
            validator.validatePlayerStrategyInstance(prototype);
            plugins.add(toPlugin(jarPath, strategyClass, prototype));
        }

        if (plugins.isEmpty()) {
            throw new PluginValidationException("No PlayerStrategy implementation found in plugin JAR: " + jarPath);
        }
        return List.copyOf(plugins);
    }

    private URLClassLoader createClassLoader(Path jarPath) {
        try {
            URL[] urls = { jarPath.toUri().toURL() };
            ClassLoader parent = Thread.currentThread().getContextClassLoader();
            if (parent == null) {
                parent = PlayerStrategy.class.getClassLoader();
            }
            return new URLClassLoader(urls, parent);
        } catch (MalformedURLException e) {
            throw new PluginLoadException("Invalid bot plugin JAR URL: " + jarPath, e);
        }
    }

    private Class<?> loadClass(ClassLoader classLoader, String className) {
        try {
            return Class.forName(className, false, classLoader);
        } catch (ClassNotFoundException | LinkageError e) {
            throw new PluginLoadException("Unable to load bot plugin class: " + className, e);
        }
    }

    private PlayerStrategy instantiate(Class<? extends PlayerStrategy> strategyClass) {
        try {
            Constructor<? extends PlayerStrategy> constructor = strategyClass.getDeclaredConstructor();
            constructor.setAccessible(true);
            return constructor.newInstance();
        } catch (InstantiationException | IllegalAccessException | InvocationTargetException | NoSuchMethodException e) {
            throw new PluginLoadException("Unable to instantiate bot strategy: " + strategyClass.getName(), e);
        }
    }

    private BotPlugin toPlugin(Path jarPath, Class<? extends PlayerStrategy> strategyClass, PlayerStrategy prototype) {
        PluginMetadata metadata = new PluginMetadata(
                prototype.getStrategyId(),
                prototype.getStrategyName(),
                prototype.getDescription(),
                strategyClass.getName(),
                jarPath.toAbsolutePath().normalize().toString(),
                Instant.now()
        );
        return new BotPlugin(metadata, strategyClass);
    }
}
