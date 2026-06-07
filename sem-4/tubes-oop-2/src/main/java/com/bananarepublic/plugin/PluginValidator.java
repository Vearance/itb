package com.bananarepublic.plugin;

import com.bananarepublic.core.card.AbstractExperimentCard;
import com.bananarepublic.core.card.CardType;
import com.bananarepublic.core.card.ExperimentCard;
import com.bananarepublic.core.bot.PlayerStrategy;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.Player;
import com.bananarepublic.exception.plugin.InvalidPluginException;
import com.bananarepublic.exception.plugin.PluginValidationException;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Objects;

public final class PluginValidator {
    public void validateJarPath(Path jarPath) {
        Objects.requireNonNull(jarPath, "jarPath");
        if (!Files.isRegularFile(jarPath)) {
            throw new InvalidPluginException("Plugin path is not a file: " + jarPath);
        }
        if (!jarPath.getFileName().toString().toLowerCase().endsWith(".jar")) {
            throw new InvalidPluginException("Plugin file must use the .jar extension");
        }
    }

    public void validateExperimentCardClass(Class<?> pluginClass) {
        Objects.requireNonNull(pluginClass, "pluginClass");
        if (!ExperimentCard.class.isAssignableFrom(pluginClass)) {
            throw new InvalidPluginException("Class does not implement ExperimentCard: " + pluginClass.getName());
        }
        int modifiers = pluginClass.getModifiers();
        if (pluginClass.isInterface() || Modifier.isAbstract(modifiers)) {
            throw new InvalidPluginException("ExperimentCard plugin must be a concrete class: " + pluginClass.getName());
        }
        try {
            pluginClass.getDeclaredConstructor();
        } catch (NoSuchMethodException e) {
            throw new PluginValidationException("ExperimentCard plugin must have a no-argument constructor: " + pluginClass.getName(), e);
        }
    }

    public void validateExperimentCardInstance(ExperimentCard card) {
        Objects.requireNonNull(card, "card");
        if (card.getCardName() == null || card.getCardName().isBlank()) {
            throw new PluginValidationException("Plugin card name cannot be blank: " + card.getClass().getName());
        }
        if (card.getDescription() == null || card.getDescription().isBlank()) {
            throw new PluginValidationException("Plugin card description cannot be blank: " + card.getClass().getName());
        }
        if (card.getId() == null || card.getId().value().isBlank()) {
            throw new PluginValidationException("Plugin card id cannot be blank: " + card.getClass().getName());
        }
        if (!card.getId().value().startsWith("PLUGIN-")) {
            throw new PluginValidationException("Plugin card id must start with PLUGIN-: " + card.getId().value());
        }
        if (card.getType() != CardType.PLUGIN) {
            throw new PluginValidationException("Plugin card type must be PLUGIN: " + card.getClass().getName());
        }
        validatePolymorphicEffect(card.getClass());
    }

    public void validatePlayerStrategyClass(Class<?> pluginClass) {
        Objects.requireNonNull(pluginClass, "pluginClass");
        if (!PlayerStrategy.class.isAssignableFrom(pluginClass)) {
            throw new InvalidPluginException("Class does not implement PlayerStrategy: " + pluginClass.getName());
        }
        int modifiers = pluginClass.getModifiers();
        if (pluginClass.isInterface() || Modifier.isAbstract(modifiers)) {
            throw new InvalidPluginException("PlayerStrategy plugin must be a concrete class: " + pluginClass.getName());
        }
        try {
            pluginClass.getDeclaredConstructor();
        } catch (NoSuchMethodException e) {
            throw new PluginValidationException("PlayerStrategy plugin must have a no-argument constructor: " + pluginClass.getName(), e);
        }
    }

    public void validatePlayerStrategyInstance(PlayerStrategy strategy) {
        Objects.requireNonNull(strategy, "strategy");
        if (strategy.getStrategyId() == null || strategy.getStrategyId().isBlank()) {
            throw new PluginValidationException("Bot strategy id cannot be blank: " + strategy.getClass().getName());
        }
        if (!strategy.getStrategyId().startsWith("BOT-")) {
            throw new PluginValidationException("Bot strategy id must start with BOT-: " + strategy.getStrategyId());
        }
        if (strategy.getStrategyName() == null || strategy.getStrategyName().isBlank()) {
            throw new PluginValidationException("Bot strategy name cannot be blank: " + strategy.getClass().getName());
        }
        if (strategy.getDescription() == null || strategy.getDescription().isBlank()) {
            throw new PluginValidationException("Bot strategy description cannot be blank: " + strategy.getClass().getName());
        }
    }

    private void validatePolymorphicEffect(Class<?> pluginClass) {
        try {
            Method method = pluginClass.getMethod("applyEffect", GameState.class, Player.class);
            if (method.getDeclaringClass().equals(AbstractExperimentCard.class)) {
                throw new PluginValidationException("Plugin card must override applyEffect(GameState, Player): " + pluginClass.getName());
            }
        } catch (NoSuchMethodException e) {
            throw new PluginValidationException("Plugin card must define applyEffect(GameState, Player): " + pluginClass.getName(), e);
        }
    }
}
