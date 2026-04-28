#include "RailroadLoader.hpp"
#include <fstream>
#include <stdexcept>

using namespace std;

RailroadLoader::RailroadLoader(const string& file) : Loader(file) {}

RailroadLoader::~RailroadLoader() {}

void RailroadLoader::loadConfig() {
    ifstream in(filename);
    if (!in) throw runtime_error("Cannot open " + filename);
    int count, rent;
    while (in >> count >> rent) {
        if (static_cast<int>(rentTable.size()) < count) {
            rentTable.resize(count, 0);
        }
        rentTable[count - 1] = rent;
    }
}

const vector<int>& RailroadLoader::getRentTable() const {
    return rentTable;
}
