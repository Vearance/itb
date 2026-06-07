package com.bananarepublic.core.resource;

import java.util.Collections;
import java.util.EnumMap;
import java.util.Map;
import java.util.Objects;

public final class ResourceBundle {
    private final EnumMap<ResourceType, Integer> amounts;

    public ResourceBundle() {
        this.amounts = emptyAmounts();
    }

    public ResourceBundle(Map<ResourceType, Integer> amounts) {
        this.amounts = emptyAmounts();
        amounts.forEach(this::set);
    }

    public static ResourceBundle empty() {
        return new ResourceBundle();
    }

    /*
        Example of creating a ResourceBundle:
        ResourceBundle bundle = ResourceBundle.of(
            ResourceType.WOOD, 1,
            ResourceType.BRICK, 1
        );
    */ 
    public static ResourceBundle of(Object... pairs) {
        if (pairs.length % 2 != 0) {
            throw new IllegalArgumentException("ResourceBundle.of requires resource/count pairs");
        }

        ResourceBundle bundle = new ResourceBundle();
        for (int i = 0; i < pairs.length; i += 2) {
            if (!(pairs[i] instanceof ResourceType type) || !(pairs[i + 1] instanceof Integer amount)) {
                throw new IllegalArgumentException("Pairs must be ResourceType followed by Integer amount");
            }
            bundle.set(type, amount);
        }
        return bundle;
    }

    public int get(ResourceType type) {
        return amounts.get(Objects.requireNonNull(type, "type"));
    }

    public void set(ResourceType type, int amount) {
        requireNonNegative(amount);
        amounts.put(Objects.requireNonNull(type, "type"), amount);
    }

    public void add(ResourceType type, int amount) {
        requireNonNegative(amount);
        set(type, get(type) + amount);
    }

    public void remove(ResourceType type, int amount) {
        requireNonNegative(amount);
        if (get(type) < amount) {
            throw new IllegalArgumentException("Not enough " + type + " resources");
        }
        set(type, get(type) - amount);
    }

    public boolean hasAtLeast(ResourceBundle required) {
        for (ResourceType type : ResourceType.values()) {
            if (get(type) < required.get(type)) {
                return false;
            }
        }
        return true;
    }

    public void addBundle(ResourceBundle other) {
        for (ResourceType type : ResourceType.values()) {
            add(type, other.get(type));
        }
    }

    public void removeBundle(ResourceBundle other) {
        if (!hasAtLeast(other)) {
            throw new IllegalArgumentException("Not enough resources");
        }
        for (ResourceType type : ResourceType.values()) {
            remove(type, other.get(type));
        }
    }

    public int total() {
        int total = 0;
        for (int amount : amounts.values()) {
            total += amount;
        }
        return total;
    }

    public Map<ResourceType, Integer> asMap() {
        return Collections.unmodifiableMap(amounts);
    }

    public ResourceBundle copy() {
        return new ResourceBundle(amounts);
    }

    @Override
    public boolean equals(Object other) {
        if (this == other) {
            return true;
        }
        if (!(other instanceof ResourceBundle bundle)) {
            return false;
        }
        return amounts.equals(bundle.amounts);
    }

    @Override
    public int hashCode() {
        return amounts.hashCode();
    }

    private static EnumMap<ResourceType, Integer> emptyAmounts() {
        EnumMap<ResourceType, Integer> map = new EnumMap<>(ResourceType.class);
        for (ResourceType type : ResourceType.values()) {
            map.put(type, 0);
        }
        return map;
    }

    private static void requireNonNegative(int amount) {
        if (amount < 0) {
            throw new IllegalArgumentException("Resource amount cannot be negative");
        }
    }
}
