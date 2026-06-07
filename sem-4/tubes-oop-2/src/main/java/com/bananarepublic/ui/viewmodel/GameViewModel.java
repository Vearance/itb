package com.bananarepublic.ui.viewmodel;

import com.bananarepublic.core.game.GamePhase;
import javafx.beans.property.*;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;

public final class GameViewModel {
    private final BoardViewModel board = new BoardViewModel();
    private final ObservableList<PlayerViewModel> players = FXCollections.observableArrayList();
    private final ObservableList<ResourceViewModel> resources = FXCollections.observableArrayList();
    private final ObservableList<CardViewModel> cards = FXCollections.observableArrayList();
    private final TimerViewModel timer = new TimerViewModel();
    private final StringProperty currentPlayerName = new SimpleStringProperty("-");
    private final ObjectProperty<GamePhase> phase = new SimpleObjectProperty<>();
    private final StringProperty logText = new SimpleStringProperty("");
    public BoardViewModel getBoard(){return board;} public ObservableList<PlayerViewModel> getPlayers(){return players;} public ObservableList<ResourceViewModel> getResources(){return resources;} public ObservableList<CardViewModel> getCards(){return cards;} public TimerViewModel getTimer(){return timer;}
    public StringProperty currentPlayerNameProperty(){return currentPlayerName;} public void setCurrentPlayerName(String n){currentPlayerName.set(n);} public ObjectProperty<GamePhase> phaseProperty(){return phase;} public void setPhase(GamePhase p){phase.set(p);} public StringProperty logTextProperty(){return logText;} public boolean hasLog(){return logText.get()!=null&&!logText.get().isBlank();} public void appendLog(String line){logText.set((logText.get()==null?"":logText.get()) + line + "\n");}
}
