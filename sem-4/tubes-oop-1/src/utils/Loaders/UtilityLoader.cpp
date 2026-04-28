#include "UtilityLoader.hpp"
#include <fstream>
#include <stdexcept>

using namespace std;

UtilityLoader::UtilityLoader(const string& file) : Loader(file) {}

UtilityLoader::~UtilityLoader() {}

void UtilityLoader::loadConfig() {
    ifstream in(filename);
    if (!in) {
        throw runtime_error("Cannot open " + filename);
    }
    int count, multiplier;
    while (in >> count >> multiplier) {
        if (static_cast<int>(multipliers.size()) < count) {
            multipliers.resize(count, 0);
        }
        multipliers[count - 1] = multiplier;
    }
}

const vector<int>& UtilityLoader::getMultipliers() const {
    return multipliers;
}
