package com.bananarepublic.core.player;

import com.bananarepublic.core.resource.ResourceType;

import java.util.Objects;

public abstract class AbstractPlayer implements Player {
    private final PlayerId id;
    private final String name;
    private final PlayerColor color;
    private final PlayerInventory inventory;
    private final PlayerCardHand cardHand;
    private final PlayerScore score;
    private final PlayerSupply supply;
    private final PlayerStats stats;

    protected AbstractPlayer(PlayerId id, String name, PlayerColor color) {
        if (name == null || name.isBlank()) {
            throw new IllegalArgumentException("Player name cannot be blank");
        }
        this.id = Objects.requireNonNull(id, "id");
        this.name = name;
        this.color = Objects.requireNonNull(color, "color");
        this.inventory = new PlayerInventory();
        this.cardHand = new PlayerCardHand();
        this.score = new PlayerScore();
        this.supply = new PlayerSupply();
        this.stats = new PlayerStats();
    }

    public PlayerId getId() {
        return id;
    }

    @Override
    public String getName() {
        return name;
    }

    public PlayerColor getColor() {
        return color;
    }

    public PlayerInventory getInventory() {
        return inventory;
    }

    public PlayerCardHand getCardHand() {
        return cardHand;
    }

    public PlayerScore getScore() {
        return score;
    }

    public PlayerSupply getSupply() {
        return supply;
    }

    public PlayerStats getStats() {
        return stats;
    }

    @Override
    public int getResourceCount(ResourceType type) {
        return inventory.getResourceCount(type);
    }

    @Override
    public void addResource(ResourceType type, int amount) {
        inventory.addResource(type, amount);
    }

    @Override
    public void removeResource(ResourceType type, int amount) {
        inventory.spendResource(type, amount);
    }
}
