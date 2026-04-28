#pragma once
#include "Card.hpp"

/// @brief Chance card effect types defined by the spec.
enum class ChanceEffect {
    GO_TO_NEAREST_STATION, ///< "Pergi ke stasiun terdekat."
    MOVE_BACK_3,           ///< "Mundur 3 petak."
    GO_TO_JAIL,            ///< "Masuk Penjara."
    GET_OUT_OF_JAIL        ///< "Bebas dari Penjara."
};

/// @brief Chance card drawn when a player lands on a Kesempatan tile.
class ChanceCard : public Card {
private:
    ChanceEffect effect;

public:
    ChanceCard(const std::string& name, const std::string& description, ChanceEffect effect);
    ~ChanceCard() override;

    ChanceEffect getEffect() const;

    void executeAction(IGameContext& ctx) override;
};
