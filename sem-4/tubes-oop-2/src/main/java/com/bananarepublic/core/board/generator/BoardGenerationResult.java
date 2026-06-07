package com.bananarepublic.core.board.generator;

import com.bananarepublic.core.board.Board;

import java.util.List;

public record BoardGenerationResult(Board board, List<String> validationErrors) {
    public boolean isValid() {
        return validationErrors.isEmpty();
    }
}
