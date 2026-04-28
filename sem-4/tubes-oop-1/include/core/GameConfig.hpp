#pragma once
#include <vector>

/**
 * @brief Value object holding all game configuration loaded from config files.
 * Populated by ConfigManager; passed around via const-ref.
 */
class GameConfig {
public:
    int goSalary = 200;
    int jailFine = 50;

    int maxTurn = 15;
    int startingBalance = 1000;

    int pphFlat = 150;
    int pphPercentage = 10;
    int pbmFlat = 200;

    std::vector<int> railroadRentTable = {25, 50, 100, 200};

    std::vector<int> utilityMultipliers = {4, 10};
};
