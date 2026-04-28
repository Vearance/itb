#pragma once
#include "SpecialTile.hpp"

/// @brief Jail tile — visiting player is unaffected; imprisoned player must pay fine or roll double.
class JailTile : public SpecialTile {
private:
    int fineAmount; ///< Amount from config (special.txt → JAIL_FINE)

public:
    JailTile(int id, const std::string& code, const std::string& name, int fineAmount);
    ~JailTile() override;

    int getFineAmount() const;

protected:
    void executeAction(IGameContext& ctx) override;
};
