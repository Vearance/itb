package com.bananarepublic.plugin.card;

import com.bananarepublic.core.card.ExperimentCard;
import com.bananarepublic.exception.plugin.PluginLoadException;
import com.bananarepublic.plugin.Plugin;
import com.bananarepublic.plugin.PluginMetadata;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.Objects;

public final class ExperimentCardPlugin implements Plugin {
    private final PluginMetadata metadata;
    private final Class<? extends ExperimentCard> cardClass;

    public ExperimentCardPlugin(PluginMetadata metadata, Class<? extends ExperimentCard> cardClass) {
        this.metadata = Objects.requireNonNull(metadata, "metadata");
        this.cardClass = Objects.requireNonNull(cardClass, "cardClass");
    }

    @Override
    public PluginMetadata getMetadata() {
        return metadata;
    }

    public ExperimentCard createCard() {
        try {
            Constructor<? extends ExperimentCard> constructor = cardClass.getDeclaredConstructor();
            constructor.setAccessible(true);
            return constructor.newInstance();
        } catch (InstantiationException | IllegalAccessException | InvocationTargetException | NoSuchMethodException e) {
            throw new PluginLoadException("Unable to instantiate plugin card: " + metadata.className(), e);
        }
    }
}
