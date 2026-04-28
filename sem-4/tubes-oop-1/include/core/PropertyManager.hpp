#pragma once

#include <vector>

class Player;
class Board;
class Logger;
class PropertyTile;
class StreetTile;
class IUserInteraction;

/**
 * @brief Handles property management commands: mortgage (gadai), redeem (tebus),
 * and build (bangun).
 *
 * Single Responsibility: property command logic only.
 * Extracted from Game to keep the orchestrator thin.
 */
class PropertyManager {
public:
    PropertyManager() = default;
    ~PropertyManager() = default;

    /** Mortgage a property (GADAI command). */
    void mortgage(Player& player, Board& board, int currentTurn, Logger& logger,
                  IUserInteraction* ui);

    /** Redeem a mortgaged property (TEBUS command). */
    void redeem(Player& player, int currentTurn, Logger& logger, IUserInteraction* ui);

    /** Build houses/hotels on a monopolized color group (BANGUN command). */
    void build(Player& player, Board& board, int currentTurn, Logger& logger,
               IUserInteraction* ui);
};
