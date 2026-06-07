package com.bananarepublic.exception.build;

public class BuildingLimitExceededException extends InvalidBuildException {
    public BuildingLimitExceededException(String message) {
        super(message);
    }
}
