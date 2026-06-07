package com.bananarepublic.exception.build;

import com.bananarepublic.core.board.IntersectionId;

public class IntersectionOccupiedException extends InvalidPlacementException {
    private final IntersectionId intersectionId;


    public IntersectionOccupiedException(IntersectionId intersectionID) {
        super("Intersection " + intersectionID.value() + " is already occupied.");
        this.intersectionId = intersectionID;
    }

    public IntersectionId getIntersectionId() {
        return intersectionId;
    }
    
}
