package com.bananarepublic.ui.viewmodel;

import javafx.beans.property.*;

public final class CardViewModel {
    private final StringProperty name = new SimpleStringProperty();
    private final StringProperty description = new SimpleStringProperty();
    private final BooleanProperty playable = new SimpleBooleanProperty(true);
    public CardViewModel(String name, String description){this.name.set(name);this.description.set(description);}    
    public String getName(){return name.get();} public String getDescription(){return description.get();} public boolean isPlayable(){return playable.get();}
    public StringProperty nameProperty(){return name;} public StringProperty descriptionProperty(){return description;} public BooleanProperty playableProperty(){return playable;}
}
