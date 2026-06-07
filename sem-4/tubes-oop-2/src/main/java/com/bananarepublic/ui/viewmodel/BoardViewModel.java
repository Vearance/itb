package com.bananarepublic.ui.viewmodel;

import javafx.collections.FXCollections;
import javafx.collections.ObservableList;

public final class BoardViewModel {
    private final ObservableList<HexTileViewModel> hexTiles = FXCollections.observableArrayList();
    public ObservableList<HexTileViewModel> getHexTiles(){return hexTiles;}
}
