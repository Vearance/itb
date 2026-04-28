#pragma once
#include "Loader.hpp"

/// @brief Loads special tile config from special.txt (GO salary, jail fine).
class SpecialLoader : public Loader {
private:
    int goSalary = 200;
    int jailFine = 50;

public:
    explicit SpecialLoader(const std::string& filename);
    ~SpecialLoader() override;

    void loadConfig() override;

    int getGoSalary() const;
    int getJailFine() const;
};
