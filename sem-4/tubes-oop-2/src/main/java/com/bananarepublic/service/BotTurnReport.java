package com.bananarepublic.service;

import java.util.List;

public record BotTurnReport(List<String> messages, boolean endedTurn) {
    public BotTurnReport {
        messages = messages == null ? List.of() : List.copyOf(messages);
    }
}
