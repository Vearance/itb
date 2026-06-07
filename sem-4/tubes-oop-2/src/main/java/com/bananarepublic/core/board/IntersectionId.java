package com.bananarepublic.core.board;

public record IntersectionId(String value) {
    public IntersectionId {
        if (value == null || value.isBlank()) {
            throw new IllegalArgumentException("Intersection id cannot be blank");
        }
    }
}
