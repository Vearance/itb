package com.bananarepublic.ui.viewmodel;

import com.bananarepublic.core.resource.ResourceType;
import javafx.beans.property.*;

public final class ResourceViewModel {
    private final ObjectProperty<ResourceType> type = new SimpleObjectProperty<>();
    private final IntegerProperty amount = new SimpleIntegerProperty();
    public ResourceViewModel(ResourceType type, int amount){this.type.set(type);this.amount.set(amount);}    
    public ResourceType getType(){return type.get();}
    public int getAmount(){return amount.get();}
    public ObjectProperty<ResourceType> typeProperty(){return type;}
    public IntegerProperty amountProperty(){return amount;}
}
