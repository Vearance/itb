package com.bananarepublic.ui.viewmodel;

import com.bananarepublic.timer.TurnTimer;
import javafx.beans.property.*;

public final class TimerViewModel {
    private final IntegerProperty remainingSeconds = new SimpleIntegerProperty(TurnTimer.ROUND_DURATION);
    private final BooleanProperty running = new SimpleBooleanProperty(false);
    public int getRemainingSeconds(){return remainingSeconds.get();} public void setRemainingSeconds(int v){remainingSeconds.set(v);} public IntegerProperty remainingSecondsProperty(){return remainingSeconds;}
    public boolean isRunning(){return running.get();} public void setRunning(boolean v){running.set(v);} public BooleanProperty runningProperty(){return running;}
    public String format(){int s=getRemainingSeconds(); return String.format("%02d:%02d", s/60, s%60);} 
}
