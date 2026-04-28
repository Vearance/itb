#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

class Player;
class Board;
class Dice;
class Logger;
class IUserInteraction;
class IGameContext;
class PropertyManager;
class PropertyTile;
template <typename T> class CardDeck;
class SkillCard;

/**
 * @brief Dispatches player commands to the appropriate handler.
 *
 * Single Responsibility: parsing and routing user input during a turn.
 * Extracted from Game::handleCommand() to keep the orchestrator thin.
 */
class CommandHandler {
public:
    CommandHandler() = default;
    ~CommandHandler() = default;

    /**
     * @brief Parse and dispatch a single command string.
     * @param input Raw command input from the player.
     * @param player The active player issuing the command.
     * @param ctx Game context for state queries and mutations.
     * @param propMgr PropertyManager for gadai/tebus/bangun.
     * @param board Board reference for display and queries.
     * @param dice Dice reference for rolling.
     * @param skillDeck Skill card deck for discarding used cards.
     * @param ui IUserInteraction for display and input commands.
     * @param logger Logger for logging events.
     * @param lastDiceTotal Reference to the last dice total (updated on roll).
     * @param saveCallback Callback to invoke for the SIMPAN command (takes filename).
     */
    using SaveCallback = std::function<void(const std::string&)>;

    void dispatch(const std::string& input, Player& player, IGameContext& ctx,
                  PropertyManager& propMgr, Board& board, Dice& dice,
                  CardDeck<SkillCard>* skillDeck, IUserInteraction* ui, Logger& logger,
                  int& lastDiceTotal, SaveCallback saveCallback,
                  const std::vector<std::unique_ptr<Player>>& players);

private:
    /** Handle LEMPAR_DADU / ATUR_DADU commands. */
    void handleLemparDadu(Player& player, Dice& dice, IGameContext& ctx, Logger& logger,
                          int& lastDiceTotal, IUserInteraction* ui, int d1 = -1, int d2 = -1);

    /** Handle GUNAKAN_KEMAMPUAN command. */
    void handleGunakanKemampuan(Player& player, IGameContext& ctx, CardDeck<SkillCard>* skillDeck,
                                Logger& logger, IUserInteraction* ui);
};
