#pragma once
#include "SpecialTile.hpp"

/// @brief Sends the landing player directly to jail (no GO salary granted).
class GoToJailTile : public SpecialTile {
public:
    GoToJailTile(int id, const std::string& code, const std::string& name);
    ~GoToJailTile() override;

protected:
    void executeAction(IGameContext& ctx) override;
};
