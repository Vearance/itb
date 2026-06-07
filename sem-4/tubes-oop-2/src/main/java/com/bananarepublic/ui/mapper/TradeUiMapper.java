package com.bananarepublic.ui.mapper;

import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.ui.viewmodel.TradeViewModel;
import java.util.Map;

public final class TradeUiMapper {
    public TradeViewModel fromMaps(Map<ResourceType, Integer> give, Map<ResourceType, Integer> receive){
        TradeViewModel vm = new TradeViewModel(); if(give!=null) vm.getGive().putAll(give); if(receive!=null) vm.getReceive().putAll(receive); return vm;
    }
}
