package com.bananarepublic.core.board;

public final class TokenNumber {
    private final int value;

    public TokenNumber(int value) {
        if (value < 2 || value > 12 || value == 7) {
            throw new IllegalArgumentException("Token number must be 2-12 excluding 7");
        }
        this.value = value;
    }

    public int getValue() {
        return value;
    }

    public boolean isRedToken() {
        return value == 6 || value == 8;
    }
}
