package com.bananarepublic.service;

import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.ResourceChangedEvent;
import com.bananarepublic.event.ResourceStolenEvent;

import java.util.Objects;
import java.util.Optional;
import java.util.Random;

public final class StealService {
    private final GameEventBus eventBus;
    private final Random random;

    public StealService(GameEventBus eventBus) {
        this(eventBus, new Random());
    }

    public StealService(GameEventBus eventBus, Random random) {
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
        this.random = Objects.requireNonNull(random, "random");
    }

    public Optional<ResourceType> stealRandomResource(GameState state, PlayerId thiefId, PlayerId victimId) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(thiefId, "thiefId");
        Objects.requireNonNull(victimId, "victimId");
        if (thiefId.equals(victimId)) {
            throw new IllegalArgumentException("Player cannot steal from themselves");
        }

        AbstractPlayer thief = state.getPlayerById(thiefId);
        AbstractPlayer victim = state.getPlayerById(victimId);
        Optional<ResourceType> stolenType = chooseRandomResource(victim);
        if (stolenType.isEmpty()) {
            return Optional.empty();
        }

        ResourceType resourceType = stolenType.orElseThrow();
        victim.removeResource(resourceType, 1);
        thief.addResource(resourceType, 1);
        eventBus.publish(new ResourceChangedEvent(victim.getId(), resourceType, -1, victim.getResourceCount(resourceType)));
        eventBus.publish(new ResourceChangedEvent(thief.getId(), resourceType, 1, thief.getResourceCount(resourceType)));
        eventBus.publish(new ResourceStolenEvent(thief.getId(), victim.getId(), resourceType));
        return Optional.of(resourceType);
    }

    private Optional<ResourceType> chooseRandomResource(AbstractPlayer player) {
        int totalResources = player.getInventory().getTotalResourceCount();
        if (totalResources == 0) {
            return Optional.empty();
        }

        int index = random.nextInt(totalResources);
        for (ResourceType type : ResourceType.values()) {
            int amount = player.getResourceCount(type);
            if (index < amount) {
                return Optional.of(type);
            }
            index -= amount;
        }
        return Optional.empty();
    }
}
