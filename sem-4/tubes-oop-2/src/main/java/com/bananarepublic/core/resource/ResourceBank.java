package com.bananarepublic.core.resource;

import com.bananarepublic.config.ResourceConfig;

public final class ResourceBank {
    private final ResourceBundle resources;

    public ResourceBank() {
        this.resources = ResourceBundle.empty();
        for (ResourceType type : ResourceType.values()) {
            resources.set(type, ResourceConfig.INITIAL_BANK_RESOURCE_COUNT);
        }
    }

    public int getCount(ResourceType type) {
        return resources.get(type);
    }

    public boolean hasResource(ResourceType type, int amount) {
        return resources.get(type) >= amount;
    }

    public void giveResource(ResourceType type, int amount) {
        resources.remove(type, amount);
        assert resources.get(type) >= 0 : "Resource bank count cannot be negative after giving " + type;
    }

    public void receiveResource(ResourceType type, int amount) {
        resources.add(type, amount);
        assert resources.get(type) <= ResourceConfig.INITIAL_BANK_RESOURCE_COUNT
                : "Resource bank count cannot exceed finite bank size for " + type;
    }

    public boolean hasResources(ResourceBundle bundle) {
        return resources.hasAtLeast(bundle);
    }

    public void giveResourceBundle(ResourceBundle bundle) {
        resources.removeBundle(bundle);
    }

    public void receiveResourceBundle(ResourceBundle bundle) {
        resources.addBundle(bundle);
    }

    public ResourceBundle snapshot() {
        return resources.copy();
    }
}
