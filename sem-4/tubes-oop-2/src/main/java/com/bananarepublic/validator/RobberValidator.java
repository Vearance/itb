package com.bananarepublic.validator;

import com.bananarepublic.core.action.DiscardResourceAction;
import com.bananarepublic.core.action.MoveRobberAction;
import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.board.HexTile;
import com.bananarepublic.core.board.Intersection;
import com.bananarepublic.core.board.IntersectionId;
import com.bananarepublic.core.game.GamePhase;
import com.bananarepublic.core.game.GameStatus;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.exception.rule.InvalidPhaseException;
import com.bananarepublic.exception.rule.InvalidTurnException;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public final class RobberValidator {
    public static final int DISCARD_THRESHOLD = 7;

    public void validateDiscardAction(GameState state, DiscardResourceAction action) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(action, "action");
        requirePhase(state, GamePhase.DISCARD_RESOURCES);

        AbstractPlayer player = state.getPlayerById(action.getPlayerId());
        int requiredDiscardCount = getRequiredDiscardCount(player);
        if (requiredDiscardCount == 0) {
            throw new IllegalArgumentException("Player does not need to discard resources");
        }

        ResourceBundle discardedResources = action.getDiscardedResources();
        if (discardedResources.total() != requiredDiscardCount) {
            throw new IllegalArgumentException("Player must discard exactly " + requiredDiscardCount + " resources");
        }
        if (!player.getInventory().hasResources(discardedResources)) {
            throw new IllegalArgumentException("Player does not have the selected resources");
        }
    }

    public void validateMoveAfterSeven(GameState state, Board board, MoveRobberAction action) {
        validateMoveRobber(state, board, action, GamePhase.MOVE_ROBBER);
    }

    public void validateMoveFromCard(GameState state, Board board, MoveRobberAction action) {
        validateMoveRobber(state, board, action, GamePhase.PLAYER_ACTIONS);
    }

    public List<AbstractPlayer> getPlayersRequiredToDiscard(GameState state) {
        Objects.requireNonNull(state, "state");
        List<AbstractPlayer> players = new ArrayList<>();
        for (AbstractPlayer player : state.getPlayers()) {
            if (state.hasPlayerDiscarded(player.getId())) {
                continue;
            }
            if (getRequiredDiscardCount(player) > 0) {
                players.add(player);
            }
        }
        return List.copyOf(players);
    }

    public int getRequiredDiscardCount(AbstractPlayer player) {
        Objects.requireNonNull(player, "player");
        int resourceCount = player.getInventory().getTotalResourceCount();
        if (resourceCount <= DISCARD_THRESHOLD) {
            return 0;
        }
        return resourceCount / 2;
    }

    public List<AbstractPlayer> getValidStealTargets(GameState state, Board board, PlayerId thiefId, String hexTileId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(thiefId, "thiefId");
        HexTile hexTile = requireHexTile(board, hexTileId);

        List<AbstractPlayer> targets = new ArrayList<>();
        for (AbstractPlayer player : state.getPlayers()) {
            if (player.getId().equals(thiefId)) {
                continue;
            }
            if (hasBuildingAdjacentToHex(board, hexTile, player.getId())) {
                targets.add(player);
            }
        }
        return List.copyOf(targets);
    }

    private void validateMoveRobber(GameState state, Board board, MoveRobberAction action, GamePhase expectedPhase) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(action, "action");
        requirePhase(state, expectedPhase);

        if (!state.getCurrentPlayer().getId().equals(action.getPlayerId())) {
            throw new InvalidTurnException("Only the active player can move Nimon Ungu");
        }

        String targetHexTileId = action.getTargetHexTileId();
        requireHexTile(board, targetHexTileId);
        if (board.getRobber().getHexTileId().equals(targetHexTileId)) {
            throw new IllegalArgumentException("Nimon Ungu must move to a different tile");
        }

        action.getTargetPlayerId().ifPresent(targetPlayerId -> validateStealTarget(
                state,
                board,
                action.getPlayerId(),
                targetHexTileId,
                targetPlayerId
        ));
    }

    private void validateStealTarget(GameState state, Board board, PlayerId thiefId, String hexTileId, PlayerId targetPlayerId) {
        if (thiefId.equals(targetPlayerId)) {
            throw new IllegalArgumentException("Player cannot steal from themselves");
        }
        state.getPlayerById(targetPlayerId);
        HexTile hexTile = requireHexTile(board, hexTileId);
        if (!hasBuildingAdjacentToHex(board, hexTile, targetPlayerId)) {
            throw new IllegalArgumentException("Target player has no building next to the selected Nimon Ungu tile");
        }
    }

    private void requirePhase(GameState state, GamePhase expectedPhase) {
        if (state.getStatus() != GameStatus.IN_PROGRESS) {
            throw new InvalidPhaseException("Nimon Ungu action is only allowed while the game is in progress");
        }
        if (state.getPhase() != expectedPhase) {
            throw new InvalidPhaseException("Expected game phase " + expectedPhase);
        }
    }

    private HexTile requireHexTile(Board board, String hexTileId) {
        if (hexTileId == null || hexTileId.isBlank()) {
            throw new IllegalArgumentException("Hex tile id cannot be blank");
        }
        return board.getHexTile(hexTileId)
                .orElseThrow(() -> new IllegalArgumentException("Unknown hex tile: " + hexTileId));
    }

    private boolean hasBuildingAdjacentToHex(Board board, HexTile hexTile, PlayerId playerId) {
        for (IntersectionId intersectionId : hexTile.getIntersectionIds()) {
            Intersection intersection = board.getIntersections().get(intersectionId);
            if (intersection == null) {
                continue;
            }
            if (intersection.getBuilding()
                    .map(building -> building.getOwnerId().equals(playerId))
                    .orElse(false)) {
                return true;
            }
        }
        return false;
    }
}
