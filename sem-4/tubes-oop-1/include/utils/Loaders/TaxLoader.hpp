#pragma once
#include "Loader.hpp"

/// @brief Loads tax configuration from tax.txt (PPH flat, PPH percentage, PBM flat).
class TaxLoader : public Loader {
private:
    int pphFlat = 150;
    int pphPercentage = 10;
    int pbmFlat = 200;

public:
    explicit TaxLoader(const std::string& filename);
    ~TaxLoader() override;

    void loadConfig() override;

    int getPphFlat() const;
    int getPphPercentage() const;
    int getPbmFlat() const;
};
