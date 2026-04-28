#pragma once
#include "Card.hpp"

/// @brief The fixed set of Community Chest card effects defined by the spec.
enum class CommunityChestEffect {
    BIRTHDAY_COLLECT_100,
    DOCTOR_FEE_700,
    CAMPAIGN_PAY_200
};

/// @brief A community chest card that players draw when they land on a Dana Umum tile.
class CommunityChestCard : public Card {
private:
    CommunityChestEffect effect;

public:
    CommunityChestCard(const std::string& name, const std::string& description,
                       CommunityChestEffect effect);
    ~CommunityChestCard();

    /// @brief Returns the effect type of this community chest card.
    CommunityChestEffect getEffect() const;

    void executeAction(IGameContext& ctx) override;
};
