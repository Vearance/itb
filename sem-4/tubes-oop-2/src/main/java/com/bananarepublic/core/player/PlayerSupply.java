package com.bananarepublic.core.player;

import com.bananarepublic.config.Constants;
import com.bananarepublic.core.building.BuildType;

public final class PlayerSupply {
    private int settlements;
    private int laboratories;
    private int pipes;

    public PlayerSupply() {
        this(Constants.MAX_SETTLEMENT_SUPPLY, Constants.MAX_LABORATORY_SUPPLY, Constants.MAX_PIPE_SUPPLY);
    }

    public PlayerSupply(int settlements, int laboratories, int pipes) {
        requireNonNegative(settlements);
        requireNonNegative(laboratories);
        requireNonNegative(pipes);
        this.settlements = settlements;
        this.laboratories = laboratories;
        this.pipes = pipes;
    }

    public int getSettlements() {
        return settlements;
    }

    public int getLaboratories() {
        return laboratories;
    }

    public int getPipes() {
        return pipes;
    }

    public boolean hasAvailable(BuildType buildType) {
        return switch (buildType) {
            case SETTLEMENT -> settlements > 0;
            case LABORATORY -> laboratories > 0;
            case PIPE -> pipes > 0;
        };
    }

    public void use(BuildType buildType) {
        if (!hasAvailable(buildType)) {
            throw new IllegalStateException("No supply available for " + buildType);
        }
        switch (buildType) {
            case SETTLEMENT -> settlements--;
            case LABORATORY -> laboratories--;
            case PIPE -> pipes--;
        }
    }

    public void returnSettlement() {
        settlements++;
    }

    private static void requireNonNegative(int amount) {
        if (amount < 0) {
            throw new IllegalArgumentException("Supply amount cannot be negative");
        }
    }
}
