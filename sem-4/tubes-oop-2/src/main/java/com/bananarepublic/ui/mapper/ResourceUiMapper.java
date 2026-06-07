package com.bananarepublic.ui.mapper;

import com.bananarepublic.core.player.PlayerInventory;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.ui.viewmodel.ResourceViewModel;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

public final class ResourceUiMapper {
    public List<ResourceViewModel> toViewModels(PlayerInventory inventory){
        return Arrays.stream(ResourceType.values()).map(t -> new ResourceViewModel(t, inventory == null ? 0 : inventory.getResourceCount(t))).collect(Collectors.toList());
    }
}
