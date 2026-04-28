#pragma once
#include "Loader.hpp"
#include <vector>

/// @brief Loads utility multipliers from utility.txt indexed by utility count.
class UtilityLoader : public Loader {
private:
    std::vector<int> multipliers; ///< multipliers[i] = multiplier when owning (i+1) utilities

public:
    explicit UtilityLoader(const std::string& filename);
    ~UtilityLoader() override;

    void loadConfig() override;

    const std::vector<int>& getMultipliers() const;
};
