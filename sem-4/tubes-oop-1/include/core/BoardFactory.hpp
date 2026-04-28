#pragma once

#include "Board.hpp"
#include "ChanceCard.hpp"
#include "CommunityChestCard.hpp"
#include "Deck.hpp"
#include "SkillCard.hpp"
#include <memory>
#include <tuple>
#include <vector>

class GameConfig;
class ConfigManager;

/**
 * @brief Factory that builds the board, card decks, and all game objects.
 * Single Responsibility: construction of the game world from configuration.
 *
 * Extracted from Game::buildBoard() to keep Game as a thin orchestrator.
 */
class BoardFactory {
public:
    /// All objects produced by a single build().
    using BuildResult =
        std::tuple<std::unique_ptr<Board>, std::vector<std::unique_ptr<ChanceCard>>,
                   std::vector<std::unique_ptr<CommunityChestCard>>,
                   std::vector<std::unique_ptr<SkillCard>>, std::unique_ptr<CardDeck<ChanceCard>>,
                   std::unique_ptr<CardDeck<CommunityChestCard>>,
                   std::unique_ptr<CardDeck<SkillCard>>>;

    /**
     * @brief Build the board, card decks, and all cards from the given config.
     * @param cfg Fully-loaded ConfigManager.
     * @return A BuildResult containing all constructed objects.
     */
    static BuildResult build(ConfigManager& cfg);
};
