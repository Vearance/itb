package com.bananarepublic.plugin.bot;

import com.bananarepublic.core.bot.PlayerStrategy;
import com.bananarepublic.exception.plugin.PluginLoadException;
import com.bananarepublic.plugin.Plugin;
import com.bananarepublic.plugin.PluginMetadata;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.Objects;

public final class BotPlugin implements Plugin {
    private final PluginMetadata metadata;
    private final Class<? extends PlayerStrategy> strategyClass;

    public BotPlugin(PluginMetadata metadata, Class<? extends PlayerStrategy> strategyClass) {
        this.metadata = Objects.requireNonNull(metadata, "metadata");
        this.strategyClass = Objects.requireNonNull(strategyClass, "strategyClass");
    }

    @Override
    public PluginMetadata getMetadata() {
        return metadata;
    }

    public PlayerStrategy createStrategy() {
        try {
            Constructor<? extends PlayerStrategy> constructor = strategyClass.getDeclaredConstructor();
            constructor.setAccessible(true);
            return constructor.newInstance();
        } catch (InstantiationException | IllegalAccessException | InvocationTargetException | NoSuchMethodException e) {
            throw new PluginLoadException("Unable to instantiate bot strategy: " + metadata.className(), e);
        }
    }
}
