package com.bananarepublic.core.player;

import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;

public final class PlayerInventory {
    private final ResourceBundle resources = ResourceBundle.empty();

    public int getResourceCount(ResourceType type) {
        return resources.get(type);
    }

    public void addResource(ResourceType type, int amount) {
        resources.add(type, amount);
    }

    public void spendResource(ResourceType type, int amount) {
        resources.remove(type, amount);
    }

    public boolean hasResources(ResourceBundle required) {
        return resources.hasAtLeast(required);
    }

    public void addResourceBundle(ResourceBundle bundle) {
        resources.addBundle(bundle);
    }

    public void spendResourceBundle(ResourceBundle bundle) {
        resources.removeBundle(bundle);
    }

    public int getTotalResourceCount() {
        return resources.total();
    }

    public ResourceBundle snapshot() {
        return resources.copy();
    }
}
