package com.bananarepublic.plugin.card;

import com.bananarepublic.core.card.ExperimentCard;
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

public final class ExperimentCardPluginLoader implements PluginLoader<ExperimentCardPlugin> {
    private final JarClassScanner classScanner;
    private final PluginValidator validator;

    public ExperimentCardPluginLoader() {
        this(new JarClassScanner(), new PluginValidator());
    }

    public ExperimentCardPluginLoader(JarClassScanner classScanner, PluginValidator validator) {
        this.classScanner = Objects.requireNonNull(classScanner, "classScanner");
        this.validator = Objects.requireNonNull(validator, "validator");
    }

    @Override
    public List<ExperimentCardPlugin> load(Path jarPath) {
        validator.validateJarPath(jarPath);
        URLClassLoader classLoader = createClassLoader(jarPath);
        List<ExperimentCardPlugin> plugins = new ArrayList<>();

        for (String className : classScanner.scan(jarPath)) {
            Class<?> loadedClass = loadClass(classLoader, className);
            if (!ExperimentCard.class.isAssignableFrom(loadedClass)) {
                continue;
            }

            validator.validateExperimentCardClass(loadedClass);
            Class<? extends ExperimentCard> cardClass = loadedClass.asSubclass(ExperimentCard.class);
            ExperimentCard prototype = instantiate(cardClass);
            validator.validateExperimentCardInstance(prototype);
            plugins.add(toPlugin(jarPath, cardClass, prototype));
        }

        if (plugins.isEmpty()) {
            throw new PluginValidationException("No ExperimentCard implementation found in plugin JAR: " + jarPath);
        }
        return List.copyOf(plugins);
    }

    private URLClassLoader createClassLoader(Path jarPath) {
        try {
            URL[] urls = { jarPath.toUri().toURL() };
            ClassLoader parent = Thread.currentThread().getContextClassLoader();
            if (parent == null) {
                parent = ExperimentCard.class.getClassLoader();
            }
            return new URLClassLoader(urls, parent);
        } catch (MalformedURLException e) {
            throw new PluginLoadException("Invalid plugin JAR URL: " + jarPath, e);
        }
    }

    private Class<?> loadClass(ClassLoader classLoader, String className) {
        try {
            return Class.forName(className, false, classLoader);
        } catch (ClassNotFoundException | LinkageError e) {
            throw new PluginLoadException("Unable to load plugin class: " + className, e);
        }
    }

    private ExperimentCard instantiate(Class<? extends ExperimentCard> cardClass) {
        try {
            Constructor<? extends ExperimentCard> constructor = cardClass.getDeclaredConstructor();
            constructor.setAccessible(true);
            return constructor.newInstance();
        } catch (InstantiationException | IllegalAccessException | InvocationTargetException | NoSuchMethodException e) {
            throw new PluginLoadException("Unable to instantiate plugin card: " + cardClass.getName(), e);
        }
    }

    private ExperimentCardPlugin toPlugin(Path jarPath, Class<? extends ExperimentCard> cardClass, ExperimentCard prototype) {
        PluginMetadata metadata = new PluginMetadata(
                prototype.getId().value(),
                prototype.getCardName(),
                prototype.getDescription(),
                cardClass.getName(),
                jarPath.toAbsolutePath().normalize().toString(),
                Instant.now()
        );
        return new ExperimentCardPlugin(metadata, cardClass);
    }
}
