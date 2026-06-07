package com.bananarepublic.validator;

import com.bananarepublic.config.CardCostConfig;
import com.bananarepublic.core.card.CardType;
import com.bananarepublic.core.card.OwnedExperimentCard;
import com.bananarepublic.core.game.GamePhase;
import com.bananarepublic.core.game.GameStatus;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.exception.card.CardAlreadyPlayedException;
import com.bananarepublic.exception.card.DeckEmptyException;
import com.bananarepublic.exception.card.InvalidCardPlayException;
import com.bananarepublic.exception.resource.InsufficientResourceException;

import java.util.Objects;

public final class CardPlayValidator {
    public void validateCanBuyCard(GameState state) {
        Objects.requireNonNull(state, "state");
        requireCardActionPhase(state);
        if (state.getDevelopmentDeck().isEmpty()) {
            throw new DeckEmptyException("Experiment card deck is empty");
        }
        if (!state.getCurrentPlayer().getInventory().hasResources(CardCostConfig.getExperimentCardCost())) {
            throw new InsufficientResourceException("Player does not have enough resources to buy an experiment card");
        }
    }

    public void validateCanPlayCard(GameState state, AbstractPlayer player, OwnedExperimentCard ownedCard, CardType expectedType) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(player, "player");
        Objects.requireNonNull(ownedCard, "ownedCard");
        Objects.requireNonNull(expectedType, "expectedType");
        requireCardActionPhase(state);

        if (!state.getCurrentPlayer().getId().equals(player.getId())) {
            throw new InvalidCardPlayException("Only the active player can play an experiment card");
        }
        if (ownedCard.getType() != expectedType) {
            throw new InvalidCardPlayException("Expected " + expectedType + " card but got " + ownedCard.getType());
        }
        if (state.hasPlayedExperimentCardThisTurn()) {
            throw new CardAlreadyPlayedException("Player already played an experiment card this turn");
        }
        if (ownedCard.getBoughtTurnNumber() >= state.getTurnNumber()) {
            throw new CardAlreadyPlayedException("Newly bought experiment cards cannot be played this turn");
        }
    }

    private void requireCardActionPhase(GameState state) {
        if (state.getStatus() != GameStatus.IN_PROGRESS) {
            throw new InvalidCardPlayException("Experiment cards are only available while the game is in progress");
        }
        if (state.getPhase() != GamePhase.PLAYER_ACTIONS && state.getPhase() != GamePhase.ROLL_DICE) {
            throw new InvalidCardPlayException("Experiment cards can only be used during the active player's turn");
        }
    }
}
