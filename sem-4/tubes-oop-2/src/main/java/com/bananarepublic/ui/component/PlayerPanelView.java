package com.bananarepublic.ui.component;

import com.bananarepublic.core.player.PlayerColor;
import com.bananarepublic.ui.viewmodel.PlayerViewModel;
import javafx.geometry.Pos;
import javafx.scene.control.Label;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Region;
import javafx.scene.layout.VBox;

public final class PlayerPanelView extends VBox {
    public PlayerPanelView(PlayerViewModel vm) {
        getStyleClass().add(vm.isActive() ? "player-panel-active" : "player-panel");
        setSpacing(6);

        Label avatar = new Label(vm.getName().isBlank() ? "?" : vm.getName().substring(0, 1).toUpperCase());
        avatar.getStyleClass().add("player-avatar");
        avatar.setStyle("-fx-background-color: " + colorHex(vm.getColor()) + "; -fx-text-fill: " + textColor(vm.getColor()) + ";");
        Label name = new Label(vm.getName() + (vm.isActive() ? " (You)" : ""));
        name.getStyleClass().add("player-name");
        Label vp = new Label(vm.getVictoryPoints() + " VP");
        vp.getStyleClass().add("vp-badge");
        Region spacer = new Region();
        HBox.setHgrow(spacer, javafx.scene.layout.Priority.ALWAYS);
        HBox header = new HBox(8, avatar, name, spacer, vp);
        header.setAlignment(Pos.CENTER_LEFT);

        Label stats = new Label("Resources: " + vm.getResourceCount() + "    Cards: " + vm.getDevCardCount());
        stats.getStyleClass().add("player-stats");
        Label awards = new Label("Longest Road: " + vm.getLongestRoadLength()
                + "    Robber Played: " + vm.getPlayedRobberCardCount());
        awards.getStyleClass().add("player-stats");
        Label supply = new Label("Pipe: 15/15    Post: 5/5    Lab: 4/4");
        supply.getStyleClass().add("player-stats");
        getChildren().addAll(header, supply, stats, awards);
    }

    private String colorHex(PlayerColor color) {
        return switch (color) {
            case RED -> "#c61f18";
            case BLUE -> "#006aa6";
            case YELLOW -> "#eba21a";
            case WHITE -> "#fffdf8";
        };
    }

    private String textColor(PlayerColor color) {
        return color == PlayerColor.WHITE || color == PlayerColor.YELLOW ? "#4a3510" : "#ffffff";
    }
}
