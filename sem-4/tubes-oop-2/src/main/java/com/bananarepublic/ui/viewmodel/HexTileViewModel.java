package com.bananarepublic.ui.viewmodel;

import com.bananarepublic.core.board.HexTile;
import javafx.beans.property.*;

public final class HexTileViewModel {
    private final StringProperty id = new SimpleStringProperty();
    private final StringProperty terrain = new SimpleStringProperty();
    private final StringProperty token = new SimpleStringProperty();
    public HexTileViewModel(HexTile tile){ id.set(tile.getId()); terrain.set(tile.getTerrainType().name()); token.set(tile.getTokenNumber().map(t -> String.valueOf(t.getValue())).orElse("-")); }
    public String getId(){return id.get();} public String getTerrain(){return terrain.get();} public String getToken(){return token.get();}
    public StringProperty idProperty(){return id;} public StringProperty terrainProperty(){return terrain;} public StringProperty tokenProperty(){return token;}
}
