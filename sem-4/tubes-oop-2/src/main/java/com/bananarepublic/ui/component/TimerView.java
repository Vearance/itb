package com.bananarepublic.ui.component;

import com.bananarepublic.ui.viewmodel.TimerViewModel;
import javafx.beans.binding.Bindings;
import javafx.scene.control.Label;

public final class TimerView extends Label {
    public TimerView() {
        getStyleClass().add("timer-view");
    }

    public void bind(TimerViewModel vm) {
        textProperty().bind(Bindings.createStringBinding(
                () -> String.format("%02d:%02d", vm.getRemainingSeconds() / 60, vm.getRemainingSeconds() % 60),
                vm.remainingSecondsProperty()
        ));
    }
}
