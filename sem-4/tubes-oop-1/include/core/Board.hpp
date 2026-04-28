#pragma once

#include "../models/Players/Player.hpp"
#include "../models/Tiles/Tile.hpp"
#include <memory>
#include <vector>

/// @brief Manages the grid and specific tile positions of the Nimonspoli game board.
class Board {
private:
    /// @brief A collection of tiles representing the game board.
    std::vector<std::unique_ptr<Tile>> tiles;

public:
    /// @brief Creates the game board, optionally using dynamic board configuration.
    Board();

    /// @brief Destructor for the board.
    ~Board();

    /// @brief Gets the tile at the specified index or position.
    /// @param index The 0-based index of the tile on the board.
    /// @return A pointer to the requested tile.
    Tile* getTileAt(int index) const;

    /// @brief Gets a tile based on its 3-character code.
    /// @param code The unique code of the tile.
    /// @return A pointer to the requested tile, or nullptr if not found.
    Tile* getTileByCode(const std::string& code) const;

    /// @brief Calculates the new position of a player after moving a certain number of steps.
    /// @param currentPosition The current index position of the player.
    /// @param steps The number of steps to move forward (or backward if negative).
    /// @return The new target position index on the board.
    int getNewPosition(int currentPosition, int steps) const;

    /// @brief Gets the total number of tiles on the board.
    /// @return The total tile count.
    int getTotalTiles() const;

    /// @brief Gets the index position of the Jail tile.
    /// @return The zero-based index of the jail block.
    int getJailPosition() const;

    /// @brief Appends a tile to the board (used during board construction).
    void addTile(std::unique_ptr<Tile> tile);
};
