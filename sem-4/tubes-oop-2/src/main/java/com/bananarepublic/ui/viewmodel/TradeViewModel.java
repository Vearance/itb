package com.bananarepublic.ui.viewmodel;

import com.bananarepublic.core.resource.ResourceType;
import javafx.collections.FXCollections;
import javafx.collections.ObservableMap;

public final class TradeViewModel {
    private final ObservableMap<ResourceType, Integer> give = FXCollections.observableHashMap();
    private final ObservableMap<ResourceType, Integer> receive = FXCollections.observableHashMap();
    public ObservableMap<ResourceType, Integer> getGive(){return give;} public ObservableMap<ResourceType, Integer> getReceive(){return receive;}
    public void clear(){give.clear(); receive.clear();}
}
