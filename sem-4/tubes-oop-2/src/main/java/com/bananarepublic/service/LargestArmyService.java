package com.bananarepublic.service;

import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;

import java.util.Objects;
import java.util.Optional;

public final class LargestArmyService {
    private static final int MIN_LARGEST_ARMY_SIZE = 3;

    public Optional<PlayerId> recordKnightPlayed(GameState state, PlayerId playerId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(playerId, "playerId");
        state.getPlayerById(playerId).getStats().recordKnightPlayed();
        return refreshLargestArmyOwner(state);
    }

    public Optional<PlayerId> refreshLargestArmyOwner(GameState state) {
        Objects.requireNonNull(state, "state");

        PlayerId currentOwnerId = state.getLargestArmyOwnerId();
        if (currentOwnerId != null) {
            AbstractPlayer currentOwner = state.getPlayerById(currentOwnerId);
            int currentKnights = currentOwner.getStats().getPlayedKnightCount();
            if (currentOwner.getStats().hasPlayedAtLeastKnights(MIN_LARGEST_ARMY_SIZE)) {
                PlayerId ownerId = currentOwnerId;
                int ownerKnights = currentKnights;
                for (AbstractPlayer player : state.getPlayers()) {
                    int playerKnights = player.getStats().getPlayedKnightCount();
                    if (playerKnights > ownerKnights) {
                        ownerId = player.getId();
                        ownerKnights = playerKnights;
                    }
                }
                state.setLargestArmyOwnerId(ownerId);
                return Optional.of(ownerId);
            }
        }

        return chooseUniqueLeader(state, MIN_LARGEST_ARMY_SIZE)
                .map(ownerId -> {
                    state.setLargestArmyOwnerId(ownerId);
                    return ownerId;
                })
                .or(() -> {
                    state.clearLargestArmyOwner();
                    return Optional.empty();
                });
    }

    private Optional<PlayerId> chooseUniqueLeader(GameState state, int minimumKnights) {
        PlayerId leaderId = null;
        int leaderKnights = minimumKnights - 1;
        boolean tied = false;

        for (AbstractPlayer player : state.getPlayers()) {
            int knights = player.getStats().getPlayedKnightCount();
            if (!player.getStats().hasPlayedAtLeastKnights(minimumKnights)) {
                continue;
            }
            if (knights > leaderKnights) {
                leaderId = player.getId();
                leaderKnights = knights;
                tied = false;
            } else if (knights == leaderKnights) {
                tied = true;
            }
        }

        if (leaderId == null || tied) {
            return Optional.empty();
        }
        return Optional.of(leaderId);
    }
}
