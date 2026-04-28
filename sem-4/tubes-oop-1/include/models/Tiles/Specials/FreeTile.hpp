#pragma once
#include "SpecialTile.hpp"

/// @brief Free Parking — no effect when landed on.
class FreeTile : public SpecialTile {
public:
    FreeTile(int id, const std::string& code, const std::string& name);
    ~FreeTile() override;

protected:
    void executeAction(IGameContext& ctx) override;
};
