#pragma once
#include "SpecialTile.hpp"

/// @brief Starting tile. Grants GO salary whenever a player passes or lands here.
class GoTile : public SpecialTile {
private:
    int salary; ///< Amount credited from config (special.txt → GO_SALARY)

public:
    GoTile(int id, const std::string& code, const std::string& name, int salary);
    ~GoTile() override;

    int getSalary() const;

protected:
    void executeAction(IGameContext& ctx) override;
};
