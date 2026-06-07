package com.bananarepublic.core.board;

import java.util.Optional;

import com.bananarepublic.core.resource.ResourceType;

public enum HarborType {
    UMUM_3_1(null, 3),
    KAYU_2_TO_1(ResourceType.KAYU, 2),
    BATU_BATA_2_TO_1(ResourceType.BATU_BATA, 2),
    GANDUM_2_TO_1(ResourceType.GANDUM, 2),
    BIJIH_2_TO_1(ResourceType.BIJIH, 2),
    PISANG_2_TO_1(ResourceType.PISANG, 2);

    private final ResourceType resourceType;
    private final int ratio;

    HarborType(ResourceType resourceType, int ratio) {
        this.resourceType = resourceType;
        this.ratio = ratio;
    }

    public Optional<ResourceType> getResourceType() {
        return Optional.ofNullable(resourceType);
    }

    public int getRatio() {
        return ratio;
    }
}
