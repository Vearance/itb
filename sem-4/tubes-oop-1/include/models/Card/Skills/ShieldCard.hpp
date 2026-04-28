#pragma once
#include "SkillCard.hpp"

/// @brief A card that protects the player from rent and negative effects for 1 turn.
class ShieldCard : public SkillCard {
private:
    /// @brief The duration of the shield effect in turns.
    int duration;

public:
    /// @brief Creates a shield card with the given configuration.
    ShieldCard(const std::string& name, const std::string& description, int duration);
    ~ShieldCard();

    /// @brief Gets the duration (in turns) of the shield effect.
    int getDuration() const;

    void executeAction(IGameContext& ctx) override;
};
