#pragma once
#include "Loader.hpp"
#include <string>
#include <tuple>
#include <vector>

using ActionTileEntry = std::tuple<int, std::string, std::string, std::string>;

/// @brief Loads non-property board tiles from action.txt.
class ActionLoader : public Loader {
private:
    std::vector<ActionTileEntry> actionTiles;
    std::vector<ActionTileEntry> specialTiles;

public:
    explicit ActionLoader(const std::string& filename);
    ~ActionLoader() override;

    void loadConfig() override;

    const std::vector<ActionTileEntry>& getActionTiles() const;
    const std::vector<ActionTileEntry>& getSpecialTiles() const;
};