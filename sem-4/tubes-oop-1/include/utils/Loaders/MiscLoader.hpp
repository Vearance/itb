#pragma once
#include "Loader.hpp"

/// @brief Loads miscellaneous config from misc.txt (MAX_TURN, starting balance).
class MiscLoader : public Loader {
private:
    int maxTurn = 15;
    int startingBalance = 1000;

public:
    explicit MiscLoader(const std::string& filename);
    ~MiscLoader() override;

    void loadConfig() override;

    int getMaxTurn() const;
    int getStartingBalance() const;
};
