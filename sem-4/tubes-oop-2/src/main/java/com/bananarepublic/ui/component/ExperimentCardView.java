package com.bananarepublic.ui.component;

import com.bananarepublic.ui.viewmodel.CardViewModel;
import javafx.scene.control.Label;
import javafx.scene.layout.VBox;

public final class ExperimentCardView extends VBox {
    public ExperimentCardView(CardViewModel vm){ getStyleClass().add("experiment-card"); Label title=new Label(vm.getName()); title.getStyleClass().add("card-title"); getChildren().addAll(title,new Label(vm.getDescription())); }
}
