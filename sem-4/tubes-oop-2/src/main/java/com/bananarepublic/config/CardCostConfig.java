package com.bananarepublic.config;

import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;

public final class CardCostConfig {
    private CardCostConfig() {
    }

    public static ResourceBundle getExperimentCardCost() {
        return ResourceBundle.of(
                ResourceType.BIJIH, 1,
                ResourceType.PISANG, 1,
                ResourceType.GANDUM, 1
        );
    }
}
