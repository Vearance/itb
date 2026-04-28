#pragma once
#include "Loader.hpp"
#include "PropertyTile.hpp"
#include <memory>
#include <vector>

/// @brief Loads StreetTile, RailroadTile, and UtilityTile instances from property.txt.
class PropertyLoader : public Loader {
private:
    std::vector<std::unique_ptr<PropertyTile>> properties;

public:
    explicit PropertyLoader(const std::string& filename);
    ~PropertyLoader() override;

    void loadConfig() override;

    std::vector<std::unique_ptr<PropertyTile>>& getProperties();
};
