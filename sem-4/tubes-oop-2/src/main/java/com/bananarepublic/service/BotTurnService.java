package com.bananarepublic.service;

import com.bananarepublic.core.action.MoveRobberAction;
import com.bananarepublic.core.bot.Action;
import com.bananarepublic.core.bot.PlayerStrategy;
import com.bananarepublic.core.board.HexTile;
import com.bananarepublic.core.dice.DiceRoll;
import com.bananarepublic.core.game.GameEngine;
import com.bananarepublic.core.game.GamePhase;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public final class BotTurnService {
    private static final int MAX_ACTIONS_PER_TURN = 8;

    public BotTurnReport playTurn(GameEngine engine, PlayerStrategy strategy) {
        Objects.requireNonNull(engine, "engine");
        Objects.requireNonNull(strategy, "strategy");

        GameState state = engine.getState();
        List<String> messages = new ArrayList<>();
        AbstractPlayer bot = state.getCurrentPlayer();

        if (state.getPhase() == GamePhase.ROLL_DICE) {
            DiceRoll diceRoll = engine.rollDice();
            messages.add(bot.getName() + " (bot) roll dadu: "
                    + diceRoll.getFirstDice().getValue()
                    + " + "
                    + diceRoll.getSecondDice().getValue()
                    + " = "
                    + diceRoll.getTotal() + ".");
        }

        if (state.getPhase() == GamePhase.DISCARD_RESOURCES) {
            discardRequiredResources(engine, messages);
        }

        if (state.getPhase() == GamePhase.MOVE_ROBBER) {
            moveRobber(engine, messages);
        }

        boolean endedTurn = false;
        if (state.getPhase() == GamePhase.PLAYER_ACTIONS) {
            endedTurn = executeStrategyActions(engine, strategy, messages);
        }

        return new BotTurnReport(messages, endedTurn);
    }

    private void discardRequiredResources(GameEngine engine, List<String> messages) {
        for (AbstractPlayer player : engine.getPlayersRequiredToDiscard()) {
            int requiredCount = engine.getRequiredDiscardCount(player);
            ResourceBundle discarded = chooseDiscardBundle(player, requiredCount);
            engine.discardResources(player.getId(), discarded);
            messages.add(player.getName() + " membuang " + requiredCount + " resource karena dadu 7.");
        }
    }

    private ResourceBundle chooseDiscardBundle(AbstractPlayer player, int requiredCount) {
        ResourceBundle discarded = ResourceBundle.empty();
        int remaining = requiredCount;
        for (ResourceType type : ResourceType.values()) {
            if (remaining == 0) {
                break;
            }
            int amount = Math.min(player.getResourceCount(type), remaining);
            if (amount > 0) {
                discarded.set(type, amount);
                remaining -= amount;
            }
        }
        return discarded;
    }

    private void moveRobber(GameEngine engine, List<String> messages) {
        GameState state = engine.getState();
        String targetHexTileId = chooseRobberDestination(engine);
        PlayerId targetPlayerId = engine.getValidRobberStealTargets(targetHexTileId).stream()
                .filter(player -> player.getInventory().getTotalResourceCount() > 0)
                .map(AbstractPlayer::getId)
                .findFirst()
                .orElse(null);
        engine.moveRobber(new MoveRobberAction(state.getCurrentPlayer().getId(), targetHexTileId, targetPlayerId));
        messages.add(state.getCurrentPlayer().getName() + " (bot) memindahkan Nimon Ungu ke " + targetHexTileId + ".");
    }

    private String chooseRobberDestination(GameEngine engine) {
        String currentHexTileId = engine.getBoard().getRobber().getHexTileId();
        return engine.getBoard().getHexTiles().values().stream()
                .map(HexTile::getId)
                .filter(id -> !id.equals(currentHexTileId))
                .findFirst()
                .orElseThrow(() -> new IllegalStateException("Tidak ada petak valid untuk Nimon Ungu"));
    }

    private boolean executeStrategyActions(GameEngine engine, PlayerStrategy strategy, List<String> messages) {
        GameState state = engine.getState();
        for (int i = 0; i < MAX_ACTIONS_PER_TURN; i++) {
            Action action = strategy.takeTurn(state);
            if (action == null || action.getType() == com.bananarepublic.core.bot.ActionType.NO_OP) {
                messages.add(state.getCurrentPlayer().getName() + " (bot) tidak memilih aksi lanjutan.");
                return endTurn(engine, messages);
            }

            try {
                switch (action.getType()) {
                    case END_TURN -> {
                        return endTurn(engine, messages);
                    }
                    case BUY_EXPERIMENT_CARD -> {
                        engine.buyExperimentCard();
                        messages.add(state.getCurrentPlayer().getName() + " (bot) membeli Kartu Temuan Dr. Neroifa.");
                    }
                    case HARBOR_TRADE -> {
                        engine.tradeWithHarbor(action.getOfferedResourceType(), action.getRequestedResourceType());
                        messages.add(state.getCurrentPlayer().getName() + " (bot) trade "
                                + action.getOfferedResourceType()
                                + " ke "
                                + action.getRequestedResourceType()
                                + " lewat Terminal Dagang Gro.");
                    }
                    case NO_OP -> {
                        return endTurn(engine, messages);
                    }
                }
            } catch (RuntimeException exception) {
                messages.add(state.getCurrentPlayer().getName()
                        + " (bot) gagal menjalankan aksi "
                        + action.getType()
                        + ": "
                        + exception.getMessage());
                return endTurn(engine, messages);
            }

            if (state.getStatus() == com.bananarepublic.core.game.GameStatus.GAME_OVER) {
                return false;
            }
        }

        messages.add(state.getCurrentPlayer().getName() + " (bot) mencapai batas aksi otomatis.");
        return endTurn(engine, messages);
    }

    private boolean endTurn(GameEngine engine, List<String> messages) {
        String playerName = engine.getState().getCurrentPlayer().getName();
        engine.endTurn();
        messages.add(playerName + " (bot) mengakhiri giliran.");
        return true;
    }
}
