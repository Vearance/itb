package com.bananarepublic.core.player;

public final class PlayerScore {
    private int publicPoints;
    private int hiddenPoints;

    public int getPublicPoints() {
        return publicPoints;
    }

    public int getHiddenPoints() {
        return hiddenPoints;
    }

    public int getTotalPoints() {
        return publicPoints + hiddenPoints;
    }

    public void addPublicPoints(int points) {
        publicPoints += requireNonNegative(points);
    }

    public void removePublicPoints(int points) {
        int value = requireNonNegative(points);
        if (publicPoints < value) {
            throw new IllegalArgumentException("Public points cannot become negative");
        }
        publicPoints -= value;
    }

    public void addHiddenPoints(int points) {
        hiddenPoints += requireNonNegative(points);
    }

    private int requireNonNegative(int points) {
        if (points < 0) {
            throw new IllegalArgumentException("Score points cannot be negative");
        }
        return points;
    }
}
