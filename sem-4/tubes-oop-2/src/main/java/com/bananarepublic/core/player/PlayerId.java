package com.bananarepublic.core.player;

import java.util.Objects;

public final class PlayerId {
    private final String value;

    public PlayerId(String value) {
        if (value == null || value.isBlank()) {
            throw new IllegalArgumentException("Player id cannot be blank");
        }
        this.value = value;
    }

    public String getValue() {
        return value;
    }

    @Override
    public boolean equals(Object other) {
        if (this == other) {
            return true;
        }
        if (!(other instanceof PlayerId)) {
            return false;
        }
        PlayerId playerId = (PlayerId) other;
        return value.equals(playerId.value);
    }

    @Override
    public int hashCode() {
        return Objects.hash(value);
    }

    @Override
    public String toString() {
        return value;
    }
}
