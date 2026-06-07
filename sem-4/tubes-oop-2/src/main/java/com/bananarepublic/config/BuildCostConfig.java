package com.bananarepublic.config;

import java.util.Collections;
import java.util.EnumMap;
import java.util.Map;

import com.bananarepublic.core.building.BuildCost;
import com.bananarepublic.core.building.BuildType;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;

public final class BuildCostConfig {
    private static final Map<BuildType, BuildCost> COSTS = createCosts();

    private BuildCostConfig() {}

    public static BuildCost getCost(BuildType buildType) {
        BuildCost cost = COSTS.get(buildType);
        if (cost == null) {
            throw new IllegalArgumentException("No build cost registered for " + buildType);
        }
        return cost;
    }

    public static Map<BuildType, BuildCost> getAllCosts() {
        return COSTS;
    }

    private static Map<BuildType, BuildCost> createCosts() {
        Map<BuildType, BuildCost> costs = new EnumMap<>(BuildType.class);
        costs.put(BuildType.PIPE, new BuildCost(BuildType.PIPE, ResourceBundle.of(
                ResourceType.KAYU, 1,
                ResourceType.BATU_BATA, 1
        )));
        costs.put(BuildType.SETTLEMENT, new BuildCost(BuildType.SETTLEMENT, ResourceBundle.of(
                ResourceType.KAYU, 1,
                ResourceType.BATU_BATA, 1,
                ResourceType.PISANG, 1,
                ResourceType.GANDUM, 1
        )));
        costs.put(BuildType.LABORATORY, new BuildCost(BuildType.LABORATORY, ResourceBundle.of(
                ResourceType.BIJIH, 3,
                ResourceType.GANDUM, 2
        )));
        return Collections.unmodifiableMap(costs);
    }
}
