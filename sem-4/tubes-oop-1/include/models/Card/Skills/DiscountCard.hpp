#pragma once
#include "SkillCard.hpp"

/// @brief A card that provides a discount on property purchases for 1 turn.
class DiscountCard : public SkillCard {
private:
    /// @brief The percentage of discount provided by the card.
    int discountPercentage;

    /// @brief The duration of the discount effect in turns.
    int duration;

public:
    /// @brief Creates a discount card with the given configuration.
    /// @param name The name of the discount card.
    /// @param description The description of the discount card's effect.
    /// @param discountPercentage The percentage of discount granted by the card.
    /// @param duration The number of turns the discount effect lasts.
    DiscountCard(const std::string& name, const std::string& description, int discountPercentage,
                 int duration);
    ~DiscountCard();

    /// @brief Gets the discount percentage this card grants.
    int getDiscountPercentage() const;

    /// @brief Gets the duration (in turns) of the discount effect.
    int getDuration() const;

    void executeAction(IGameContext& ctx) override;
};
