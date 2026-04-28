#pragma once
#include "SkillCard.hpp"

/// @brief A card that allows the player to destroy an opponent's property.
class DemolitionCard : public SkillCard {
public:
    DemolitionCard(const std::string& name, const std::string& description);
    ~DemolitionCard();

    void executeAction(IGameContext& ctx) override;
};
