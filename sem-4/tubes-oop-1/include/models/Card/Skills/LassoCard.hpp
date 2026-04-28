#pragma once
#include "SkillCard.hpp"

/// @brief A card that pulls the nearest opponent in front of the player to the player's tile.
class LassoCard : public SkillCard {
public:
    LassoCard(const std::string& name, const std::string& description);
    ~LassoCard();

    void executeAction(IGameContext& ctx) override;
};
