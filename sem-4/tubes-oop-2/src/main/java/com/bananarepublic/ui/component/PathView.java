package com.bananarepublic.ui.component;

import javafx.scene.shape.Line;

public final class PathView extends Line {
    public PathView(double sx,double sy,double ex,double ey){ super(sx,sy,ex,ey); getStyleClass().add("path-view"); }
}
