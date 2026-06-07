package com.bananarepublic.service;

import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.board.HexTile;
import com.bananarepublic.core.board.Intersection;
import com.bananarepublic.core.building.AbstractBuilding;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.ResourceChangedEvent;
import com.bananarepublic.event.ResourceProducedEvent;
import com.bananarepublic.validator.DiceValidator;
import com.bananarepublic.validator.ResourceValidator;

import java.util.ArrayList;
import java.util.EnumMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

public final class ResourceProductionService {
    private final GameEventBus eventBus;

    public ResourceProductionService(GameEventBus eventBus) {
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
    }

    public List<ResourceProducedEvent> produceResources(GameState state, Board board, int diceTotal) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(board, "board");
        DiceValidator.validateDiceTotal(diceTotal);

        if (diceTotal == 7) {
            return List.of();
        }

        List<ProductionEntry> entries = collectProductionEntries(state, board, diceTotal);
        List<ResourceProducedEvent> producedEvents = new ArrayList<>();

        for (Map.Entry<ResourceType, List<ProductionEntry>> group : groupByResourceType(entries).entrySet()) {
            ResourceType resourceType = group.getKey();
            List<ProductionEntry> resourceEntries = group.getValue();
            int totalAmount = resourceEntries.stream()
                    .mapToInt(ProductionEntry::amount)
                    .sum();

            if (!ResourceValidator.canBankProduce(state.getBank(), resourceType, totalAmount)) {
                if (countAffectedPlayers(resourceEntries) > 1) {
                    continue;
                }

                int remainingBankAmount = state.getBank().getCount(resourceType);
                for (ProductionEntry entry : resourceEntries) {
                    if (remainingBankAmount == 0) {
                        break;
                    }

                    int producedAmount = Math.min(entry.amount(), remainingBankAmount);
                    producedEvents.add(payResource(state, entry, producedAmount));
                    remainingBankAmount -= producedAmount;
                }
                continue;
            }

            for (ProductionEntry entry : resourceEntries) {
                producedEvents.add(payResource(state, entry, entry.amount()));
            }
        }

        return List.copyOf(producedEvents);
    }

    private List<ProductionEntry> collectProductionEntries(GameState state, Board board, int diceTotal) {
        List<ProductionEntry> entries = new ArrayList<>();
        for (HexTile hexTile : board.getHexTiles().values()) {
            if (!producesOnRoll(hexTile, diceTotal) || isBlockedByRobber(board, hexTile)) {
                continue;
            }

            ResourceType resourceType = hexTile.getTerrainType()
                    .getProducedResource()
                    .orElseThrow(() -> new IllegalStateException("Resource tile has no produced resource"));

            for (var intersectionId : hexTile.getIntersectionIds()) {
                Intersection intersection = board.getIntersections().get(intersectionId);
                if (intersection == null) {
                    throw new IllegalStateException("Hex references missing intersection: " + intersectionId.value());
                }

                intersection.getBuilding()
                        .ifPresent(building -> entries.add(new ProductionEntry(
                                state.getPlayerById(building.getOwnerId()),
                                resourceType,
                                productionAmount(building),
                                hexTile.getId()
                        )));
            }
        }
        return entries;
    }

    private boolean producesOnRoll(HexTile hexTile, int diceTotal) {
        return hexTile.getTokenNumber()
                .map(tokenNumber -> tokenNumber.getValue() == diceTotal)
                .orElse(false);
    }

    private boolean isBlockedByRobber(Board board, HexTile hexTile) {
        return board.getRobber().getHexTileId().equals(hexTile.getId());
    }

    private int productionAmount(AbstractBuilding building) {
        return switch (building.getBuildType()) {
            case SETTLEMENT -> 1;
            case LABORATORY -> 2;
            case PIPE -> 0;
        };
    }

    private Map<ResourceType, List<ProductionEntry>> groupByResourceType(List<ProductionEntry> entries) {
        Map<ResourceType, List<ProductionEntry>> grouped = new EnumMap<>(ResourceType.class);
        for (ProductionEntry entry : entries) {
            if (entry.amount() <= 0) {
                continue;
            }
            grouped.computeIfAbsent(entry.resourceType(), ignored -> new ArrayList<>()).add(entry);
        }
        return grouped;
    }

    private int countAffectedPlayers(List<ProductionEntry> entries) {
        Set<PlayerId> playerIds = new HashSet<>();
        for (ProductionEntry entry : entries) {
            playerIds.add(entry.player().getId());
        }
        return playerIds.size();
    }

    private ResourceProducedEvent payResource(GameState state, ProductionEntry entry, int amount) {
        ResourceType resourceType = entry.resourceType();
        state.getBank().giveResource(resourceType, amount);
        entry.player().addResource(resourceType, amount);

        int newAmount = entry.player().getResourceCount(resourceType);
        ResourceProducedEvent producedEvent = new ResourceProducedEvent(
                entry.player().getId(),
                resourceType,
                amount,
                entry.hexTileId()
        );
        eventBus.publish(producedEvent);
        eventBus.publish(new ResourceChangedEvent(
                entry.player().getId(),
                resourceType,
                amount,
                newAmount
        ));
        return producedEvent;
    }

    private record ProductionEntry(AbstractPlayer player, ResourceType resourceType, int amount, String hexTileId) {
    }
}
