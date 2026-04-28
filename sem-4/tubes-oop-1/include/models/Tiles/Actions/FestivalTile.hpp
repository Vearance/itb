#pragma once
#include "ActionTile.hpp"

/// @brief Festival tile — lets the active player double rent on one owned property (up to 3×).
class FestivalTile : public ActionTile {
public:
    FestivalTile(int id, const std::string& code, const std::string& name);
    ~FestivalTile() override;

protected:
    void executeAction(IGameContext& ctx) override;
};
