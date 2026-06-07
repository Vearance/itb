package com.bananarepublic.ui.component;

import com.bananarepublic.ui.viewmodel.ResourceViewModel;
import javafx.geometry.Pos;
import javafx.scene.control.Label;
import javafx.scene.layout.VBox;

public final class ResourceCardView extends VBox {
    public ResourceCardView(ResourceViewModel vm) {
        getStyleClass().add("resource-card");
        setAlignment(Pos.CENTER);
        setSpacing(3);

        var icon = ResourceIconFactory.iconForResource(vm.getType(), 32);
        icon.getStyleClass().add("resource-card-icon");
        Label amount = new Label(String.valueOf(vm.getAmount()));
        amount.getStyleClass().add("resource-amount");
        getChildren().addAll(icon, amount);
    }
}
