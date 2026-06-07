package com.bananarepublic.core.board;

import java.util.List;

public final class Harbor {
    private final String id;
    private final HarborType type;
    private final List<IntersectionId> intersectionIds;

    public Harbor(String id, HarborType type, List<IntersectionId> intersectionIds) {
        this.id = id;
        this.type = type;
        this.intersectionIds = List.copyOf(intersectionIds);
    }

    public String getId() {
        return id;
    }

    public HarborType getType() {
        return type;
    }

    public List<IntersectionId> getIntersectionIds() {
        return intersectionIds;
    }
}
