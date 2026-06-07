package com.bananarepublic.core.board;

public record PathId(String value) {
    public PathId {
        if (value == null || value.isBlank()) {
            throw new IllegalArgumentException("Path id cannot be blank");
        }
    }
}
