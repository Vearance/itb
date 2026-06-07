package com.bananarepublic.core.player;

/*
    PlayerStats is a helper class to track history count for public-global 'awards'; which in Card implementation needed to be some kind of a Card to be given to the longest road and highest Knight card usage count.
*/
public final class PlayerStats {
    private int playedKnightCount;
    private int longestRoadLength;

    public int getPlayedKnightCount() {
        return playedKnightCount;
    }

    public int recordKnightPlayed() {
        playedKnightCount++;
        return playedKnightCount;
    }

    public int getLongestRoadLength() {
        return longestRoadLength;
    }

    public int setLongestRoadLength(int longestRoadLength) {
        if (longestRoadLength < 0) {
            throw new IllegalArgumentException("Longest road length cannot be negative");
        }
        this.longestRoadLength = longestRoadLength;
        return this.longestRoadLength;
    }

    public boolean hasPlayedAtLeastKnights(int knightCount) {
        if (knightCount < 0) {
            throw new IllegalArgumentException("Knight count cannot be negative");
        }
        return playedKnightCount >= knightCount;
    }

    public boolean hasRoadAtLeastLength(int roadLength) {
        if (roadLength < 0) {
            throw new IllegalArgumentException("Road length cannot be negative");
        }
        return longestRoadLength >= roadLength;
    }
}
