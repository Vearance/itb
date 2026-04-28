#pragma once
#include "SkillCard.hpp"

/// @brief A card that allows the player to teleport to any tile on the board.
class TeleportCard : public SkillCard {
public:
    TeleportCard(const std::string& name, const std::string& description);
    ~TeleportCard();

    void executeAction(IGameContext& ctx) override;
};
