#include "MiscLoader.hpp"
#include <fstream>
#include <stdexcept>

using namespace std;

MiscLoader::MiscLoader(const string& file) : Loader(file) {}

MiscLoader::~MiscLoader() {}

void MiscLoader::loadConfig() {
    ifstream in(filename);
    if (!in) throw runtime_error("Cannot open " + filename);
    in >> maxTurn >> startingBalance;
}

int MiscLoader::getMaxTurn() const {
    return maxTurn;
}

int MiscLoader::getStartingBalance() const {
    return startingBalance;
}
