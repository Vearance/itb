#pragma once
#include "Loader.hpp"
#include <vector>

/// @brief Loads the railroad rent table from railroad.txt into a vector indexed by railroad count.
class RailroadLoader : public Loader {
private:
    std::vector<int> rentTable; ///< rentTable[i] = rent when owning (i+1) railroads

public:
    explicit RailroadLoader(const std::string& filename);
    ~RailroadLoader() override;

    void loadConfig() override;

    const std::vector<int>& getRentTable() const;
};
