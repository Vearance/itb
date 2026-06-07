package com.bananarepublic.ui.viewmodel;

import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerColor;
import javafx.beans.property.*;

public final class PlayerViewModel {
    private final StringProperty id = new SimpleStringProperty();
    private final StringProperty name = new SimpleStringProperty();
    private final ObjectProperty<PlayerColor> color = new SimpleObjectProperty<>();
    private final IntegerProperty victoryPoints = new SimpleIntegerProperty();
    private final IntegerProperty resourceCount = new SimpleIntegerProperty();
    private final IntegerProperty devCardCount = new SimpleIntegerProperty();
    private final IntegerProperty longestRoadLength = new SimpleIntegerProperty();
    private final IntegerProperty playedRobberCardCount = new SimpleIntegerProperty();
    private final BooleanProperty active = new SimpleBooleanProperty();
    public PlayerViewModel(AbstractPlayer p, boolean active){
        this(p, active, p.getScore().getTotalPoints());
    }
    public PlayerViewModel(AbstractPlayer p, boolean active, int victoryPoints){
        id.set(p.getId().getValue()); name.set(p.getName()); color.set(p.getColor());
        this.victoryPoints.set(victoryPoints); resourceCount.set(p.getInventory().getTotalResourceCount()); devCardCount.set(p.getCardHand().getCards().size());
        longestRoadLength.set(p.getStats().getLongestRoadLength()); playedRobberCardCount.set(p.getStats().getPlayedKnightCount()); this.active.set(active);
    }
    public String getId(){return id.get();} public String getName(){return name.get();} public PlayerColor getColor(){return color.get();}
    public int getVictoryPoints(){return victoryPoints.get();} public int getResourceCount(){return resourceCount.get();} public int getDevCardCount(){return devCardCount.get();}
    public int getLongestRoadLength(){return longestRoadLength.get();} public int getPlayedRobberCardCount(){return playedRobberCardCount.get();} public boolean isActive(){return active.get();}
    public StringProperty nameProperty(){return name;} public IntegerProperty victoryPointsProperty(){return victoryPoints;} public IntegerProperty resourceCountProperty(){return resourceCount;} public IntegerProperty devCardCountProperty(){return devCardCount;}
    public IntegerProperty longestRoadLengthProperty(){return longestRoadLength;} public IntegerProperty playedRobberCardCountProperty(){return playedRobberCardCount;} public BooleanProperty activeProperty(){return active;}
}
