#include "ActionLoader.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <tuple>

using namespace std;

ActionLoader::ActionLoader(const string& file) : Loader(file) {}

ActionLoader::~ActionLoader() {}

void ActionLoader::loadConfig() {
    ifstream in(filename);
    if (!in)
        throw runtime_error("Cannot open " + filename);

    actionTiles.clear();
    specialTiles.clear();

    string line;
    while (getline(in, line)) {
        if (line.empty())
            continue;

        // Skip comments to keep action.txt extensible.
        if (line[0] == '#')
            continue;

        istringstream iss(line);

        int id;
        string code, name, type;
        string color;
        if (!(iss >> id >> code >> name >> type >> color)) {
            throw runtime_error("Invalid action tile config line: " + line);
        }

        replace(name.begin(), name.end(), '_', ' ');

        if (type == "SPESIAL") {
            specialTiles.emplace_back(id, code, name, type);
        } else if (type == "KARTU" || type == "FESTIVAL" || type == "PAJAK") {
            actionTiles.emplace_back(id, code, name, type);
        } else {
            throw runtime_error("Unknown tile type: " + type);
        }
    }
}

const vector<ActionTileEntry>& ActionLoader::getActionTiles() const {
    return actionTiles;
}

const vector<ActionTileEntry>& ActionLoader::getSpecialTiles() const {
    return specialTiles;
}