#pragma once
#include "Card.hpp"

class Player;

/// @brief A hand-held card that gives the active player a tactical advantage.
class SkillCard : public Card {
public:
    SkillCard(const std::string& name, const std::string& description);
    virtual ~SkillCard();

    virtual void executeAction(IGameContext& ctx) override = 0;

    /// @brief True if the card can be used: player has not yet rolled dice and has not
    /// used another skill card this turn.
    bool usable(const Player& player) const;
};
