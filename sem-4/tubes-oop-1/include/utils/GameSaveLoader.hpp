#pragma once
#include <string>
#include <vector>

class Game;

/**
 * @brief Handles serialization and deserialization of the full game state.
 * Single Responsibility: file I/O for save/load only.
 *
 * Format follows the spec: turn, players, turn order, property states, deck state, log.
 */
class GameSaveLoader {
public:
    GameSaveLoader();
    ~GameSaveLoader();

    /**
     * @brief Serialize the game state to a .txt file.
     * @param game The game to save.
     * @param filename Target filename (may be relative).
     * @throws FileException if the file cannot be written.
     */
    void save(const Game& game, const std::string& filename) const;

    /**
     * @brief Deserialize a .txt save file and restore game state.
     * @param game The game object to populate.
     * @param filename Source filename.
     * @throws FileException if the file is missing or malformed.
     */
    void load(Game& game, const std::string& filename) const;
};
