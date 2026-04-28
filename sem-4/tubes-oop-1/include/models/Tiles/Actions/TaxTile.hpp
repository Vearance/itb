#pragma once
#include "ActionTile.hpp"

enum class TaxType {
    PPH, ///< Pajak Penghasilan — player chooses flat or percentage
    PBM ///< Pajak Barang Mewah — fixed flat charge
};

/// @brief Tax tile. PPH lets the player choose flat vs. percentage; PBM is always flat.
class TaxTile : public ActionTile {
private:
    TaxType taxType;
    int baseAmount; ///< PPH flat amount or PBM flat amount
    int percentage; ///< PPH percentage option (only relevant for PPH)

public:
    TaxTile(int id, const std::string& code, const std::string& name,
            TaxType taxType, int baseAmount, int percentage = 0);
    ~TaxTile() override;

    TaxType getTaxType() const;
    int getBaseAmount() const;
    int getPercentage() const;

protected:
    void executeAction(IGameContext& ctx) override;
};
