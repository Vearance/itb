package com.bananarepublic.core.building;

import com.bananarepublic.core.resource.ResourceBundle;

import java.util.Objects;

public final class BuildCost {
    private final BuildType buildType;
    private final ResourceBundle resources;

    public BuildCost(BuildType buildType, ResourceBundle resources) {
        this.buildType = Objects.requireNonNull(buildType, "buildType");
        this.resources = Objects.requireNonNull(resources, "resources").copy();
    }

    public BuildType getBuildType() {
        return buildType;
    }

    public ResourceBundle getResources() {
        return resources.copy();
    }
}
