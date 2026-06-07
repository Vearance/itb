package com.bananarepublic.plugin;

import com.bananarepublic.core.card.ExperimentCard;
import com.bananarepublic.core.bot.PlayerStrategy;
import com.bananarepublic.exception.plugin.InvalidPluginException;
import com.bananarepublic.plugin.bot.BotPlugin;
import com.bananarepublic.plugin.card.ExperimentCardPlugin;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

public final class PluginRegistry {
    private final List<ExperimentCardPlugin> cardPlugins = new ArrayList<>();
    private final List<BotPlugin> botPlugins = new ArrayList<>();

    public void registerCardPlugins(List<ExperimentCardPlugin> plugins) {
        Objects.requireNonNull(plugins, "plugins");
        Set<String> ids = new HashSet<>();
        Set<String> classNames = new HashSet<>();
        for (ExperimentCardPlugin existing : cardPlugins) {
            ids.add(existing.getMetadata().id());
            classNames.add(existing.getMetadata().className());
        }
        for (ExperimentCardPlugin plugin : plugins) {
            PluginMetadata metadata = Objects.requireNonNull(plugin, "plugin").getMetadata();
            if (!ids.add(metadata.id()) || !classNames.add(metadata.className())) {
                throw new InvalidPluginException("Plugin already loaded: " + metadata.name());
            }
        }
        cardPlugins.addAll(plugins);
    }

    public List<ExperimentCardPlugin> getCardPlugins() {
        return List.copyOf(cardPlugins);
    }

    public void registerBotPlugins(List<BotPlugin> plugins) {
        Objects.requireNonNull(plugins, "plugins");
        Set<String> ids = new HashSet<>();
        Set<String> classNames = new HashSet<>();
        for (BotPlugin existing : botPlugins) {
            ids.add(existing.getMetadata().id());
            classNames.add(existing.getMetadata().className());
        }
        for (BotPlugin plugin : plugins) {
            PluginMetadata metadata = Objects.requireNonNull(plugin, "plugin").getMetadata();
            if (!ids.add(metadata.id()) || !classNames.add(metadata.className())) {
                throw new InvalidPluginException("Bot plugin already loaded: " + metadata.name());
            }
        }
        botPlugins.addAll(plugins);
    }

    public List<BotPlugin> getBotPlugins() {
        return List.copyOf(botPlugins);
    }

    public List<ExperimentCard> createCardInstances() {
        return cardPlugins.stream()
                .map(ExperimentCardPlugin::createCard)
                .toList();
    }

    public List<PlayerStrategy> createBotStrategies() {
        return botPlugins.stream()
                .map(BotPlugin::createStrategy)
                .toList();
    }

    public List<PluginMetadata> getLoadedMetadata() {
        List<PluginMetadata> metadata = new ArrayList<>();
        metadata.addAll(cardPlugins.stream()
                .map(ExperimentCardPlugin::getMetadata)
                .toList());
        metadata.addAll(botPlugins.stream()
                .map(BotPlugin::getMetadata)
                .toList());
        return List.copyOf(metadata);
    }

}
