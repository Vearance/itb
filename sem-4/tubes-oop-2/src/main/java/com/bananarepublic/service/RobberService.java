package com.bananarepublic.service;

import com.bananarepublic.core.action.MoveRobberAction;
import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.RobberMovedEvent;
import com.bananarepublic.validator.RobberValidator;

import java.util.List;
import java.util.Objects;
import java.util.Optional;

public final class RobberService {
    private final RobberValidator robberValidator;
    private final StealService stealService;
    private final GameEventBus eventBus;

    public RobberService(GameEventBus eventBus) {
        this(new RobberValidator(), new StealService(eventBus), eventBus);
    }

    public RobberService(RobberValidator robberValidator, StealService stealService, GameEventBus eventBus) {
        this.robberValidator = Objects.requireNonNull(robberValidator, "robberValidator");
        this.stealService = Objects.requireNonNull(stealService, "stealService");
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
    }

    public Optional<ResourceType> moveRobberAfterSeven(GameState state, Board board, MoveRobberAction action) {
        robberValidator.validateMoveAfterSeven(state, board, action);
        return moveRobberAndSteal(state, board, action);
    }

    public Optional<ResourceType> moveRobberFromCard(GameState state, Board board, MoveRobberAction action) {
        robberValidator.validateMoveFromCard(state, board, action);
        return moveRobberAndSteal(state, board, action);
    }

    public List<AbstractPlayer> getValidStealTargets(GameState state, Board board, PlayerId thiefId, String hexTileId) {
        return robberValidator.getValidStealTargets(state, board, thiefId, hexTileId);
    }

    public Optional<ResourceType> stealRandomResource(GameState state, PlayerId thiefId, PlayerId victimId) {
        return stealService.stealRandomResource(state, thiefId, victimId);
    }

    private Optional<ResourceType> moveRobberAndSteal(GameState state, Board board, MoveRobberAction action) {
        String previousHexTileId = board.getRobber().getHexTileId();
        board.getRobber().moveTo(action.getTargetHexTileId());
        eventBus.publish(new RobberMovedEvent(action.getPlayerId(), previousHexTileId, action.getTargetHexTileId()));

        return action.getTargetPlayerId()
                .flatMap(targetPlayerId -> stealService.stealRandomResource(state, action.getPlayerId(), targetPlayerId));
    }
}
