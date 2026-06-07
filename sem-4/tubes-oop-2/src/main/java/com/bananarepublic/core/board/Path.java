package com.bananarepublic.core.board;

import com.bananarepublic.core.building.Pipe;

import java.util.Optional;

public final class Path {
    private final PathId id;
    private final IntersectionId firstIntersectionId;
    private final IntersectionId secondIntersectionId;
    private Pipe pipe;

    public Path(PathId id, IntersectionId firstIntersectionId, IntersectionId secondIntersectionId) {
        this.id = id;
        this.firstIntersectionId = firstIntersectionId;
        this.secondIntersectionId = secondIntersectionId;
    }

    public PathId getId() {
        return id;
    }

    public IntersectionId getFirstIntersectionId() {
        return firstIntersectionId;
    }

    public IntersectionId getSecondIntersectionId() {
        return secondIntersectionId;
    }

    public boolean touches(IntersectionId intersectionId) {
        return firstIntersectionId.equals(intersectionId) || secondIntersectionId.equals(intersectionId);
    }

    public boolean hasPipe() {
        return pipe != null;
    }

    public Optional<Pipe> getPipe() {
        return Optional.ofNullable(pipe);
    }

    public void placePipe(Pipe pipe) {
        if (hasPipe()) {
            throw new IllegalStateException("Path already has a pipe");
        }
        this.pipe = pipe;
    }
}
