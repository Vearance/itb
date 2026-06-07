package com.bananarepublic.service;

import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.board.Intersection;
import com.bananarepublic.core.building.BuildType;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;

import java.util.Objects;

public final class ScoreService {
    public static final int SPECIAL_CARD_POINTS = 2;

    public int calculateTotalPoints(GameState state, Board board, AbstractPlayer player) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(player, "player");

        return calculateVisiblePoints(state, board, player)
                + player.getScore().getHiddenPoints();
    }

    public int calculateVisiblePoints(GameState state, Board board, AbstractPlayer player) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(player, "player");

        return calculateBuildingPoints(board, player.getId())
                + calculateSpecialCardPoints(state, player.getId());
    }

    public int calculateBuildingPoints(Board board, PlayerId playerId) {
        return calculateSettlementPoints(board, playerId)
                + calculateLaboratoryPoints(board, playerId);
    }

    public int calculateSettlementPoints(Board board, PlayerId playerId) {
        return calculateBuildingPoints(board, playerId, BuildType.SETTLEMENT);
    }

    public int calculateLaboratoryPoints(Board board, PlayerId playerId) {
        return calculateBuildingPoints(board, playerId, BuildType.LABORATORY);
    }

    private int calculateBuildingPoints(Board board, PlayerId playerId, BuildType buildType) {
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(playerId, "playerId");
        Objects.requireNonNull(buildType, "buildType");

        int points = 0;
        for (Intersection intersection : board.getIntersections().values()) {
            points += intersection.getBuilding()
                    .filter(building -> building.getOwnerId().equals(playerId))
                    .filter(building -> building.getBuildType() == buildType)
                    .map(building -> building.getVictoryPoints())
                    .orElse(0);
        }
        return points;
    }

    public int calculateSpecialCardPoints(GameState state, PlayerId playerId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(playerId, "playerId");

        return calculateLongestRoadPoints(state, playerId)
                + calculateLargestArmyPoints(state, playerId);
    }

    public int calculateLongestRoadPoints(GameState state, PlayerId playerId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(playerId, "playerId");

        return playerId.equals(state.getLongestRoadOwnerId()) ? SPECIAL_CARD_POINTS : 0;
    }

    public int calculateLargestArmyPoints(GameState state, PlayerId playerId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(playerId, "playerId");

        return playerId.equals(state.getLargestArmyOwnerId()) ? SPECIAL_CARD_POINTS : 0;
    }
}
