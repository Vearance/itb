package com.bananarepublic.core.card;

import java.util.ArrayList;
import java.util.List;

public final class CardFactory {
    private static final int KNIGHT_COUNT = 14;
    private static final int ROAD_BUILDING_COUNT = 3;
    private static final int MONOPOLY_COUNT = 3;
    private static final int VICTORY_POINT_COUNT = 5;

    private CardFactory() {
    }

    public static List<ExperimentCard> createStandardCards() {
        List<ExperimentCard> cards = new ArrayList<>();
        addCards(cards, CardType.KNIGHT, KNIGHT_COUNT);
        addCards(cards, CardType.ROAD_BUILDING, ROAD_BUILDING_COUNT);
        addCards(cards, CardType.MONOPOLY, MONOPOLY_COUNT);
        addCards(cards, CardType.VICTORY_POINT, VICTORY_POINT_COUNT);
        return cards;
    }

    public static ExperimentCard create(CardType type, ExperimentCardId id) {
        return switch (type) {
            case KNIGHT -> new KnightCard(id);
            case ROAD_BUILDING -> new RoadBuildingCard(id);
            case MONOPOLY -> new MonopolyCard(id);
            case VICTORY_POINT -> new VictoryPointCard(id);
            case PLUGIN -> throw new IllegalArgumentException("Plugin cards must be loaded from a plugin JAR");
        };
    }

    private static void addCards(List<ExperimentCard> cards, CardType type, int count) {
        for (int i = 1; i <= count; i++) {
            cards.add(create(type, new ExperimentCardId(type.name() + "-" + i)));
        }
    }
}
