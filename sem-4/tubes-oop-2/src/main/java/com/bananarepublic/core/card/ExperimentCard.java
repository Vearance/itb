package com.bananarepublic.core.card;

import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.Player;

public interface ExperimentCard {
    default ExperimentCardId getId() {
        return new ExperimentCardId("PLUGIN-" + sanitizeId(getClass().getName()));
    }

    default CardType getType() {
        return CardType.PLUGIN;
    }

    String getCardName();

    String getDescription();

    void applyEffect(GameState state, Player player);

    private static String sanitizeId(String raw) {
        return raw.replaceAll("[^A-Za-z0-9._-]", "_");
    }
}
