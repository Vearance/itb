package com.bananarepublic.exception.build;

import com.bananarepublic.core.board.PathId;

public class PathOccupiedException extends InvalidPlacementException {
    private final PathId pathId;

    public PathOccupiedException(PathId pathId) {
        super("Path " + pathId.value() + " is already occupied.");
        this.pathId = pathId;
    }

    public PathId getPathId() {
        return pathId;
    }
    
}
