package com.bananarepublic.core.board;

import com.bananarepublic.core.resource.ResourceType;

import java.util.Optional;

public enum TerrainType {
    HUTAN(ResourceType.KAYU),
    BUKIT(ResourceType.BATU_BATA),
    LADANG(ResourceType.GANDUM),
    GUNUNG(ResourceType.BIJIH),
    KEBUN_PISANG(ResourceType.PISANG),
    GURUN(null);

    private final ResourceType producedResource;

    TerrainType(ResourceType producedResource) {
        this.producedResource = producedResource;
    }

    public Optional<ResourceType> getProducedResource() {
        return Optional.ofNullable(producedResource);
    }

    public boolean producesResource() {
        return producedResource != null;
    }
}
