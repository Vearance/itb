#include "PropertyLoader.hpp"
#include "StreetTile.hpp"
#include "RailroadTile.hpp"
#include "UtilityTile.hpp"
#include "ColorGroup.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;

PropertyLoader::PropertyLoader(const string& file) : Loader(file) {}

PropertyLoader::~PropertyLoader() {}

void PropertyLoader::loadConfig() {
    ifstream in(filename);
    if (!in) throw runtime_error("Cannot open " + filename);

    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        istringstream iss(line);

        int id;
        string code, name, type;
        iss >> id >> code >> name >> type;

        if (type == "STREET") {
            string colorStr;
            int price, mortgage, housePrice, hotelPrice;
            iss >> colorStr >> price >> mortgage >> housePrice >> hotelPrice;

            vector<int> rentByLevel;
            string token;
            while (iss >> token) {
                if (token == "…" || token == "...") continue;
                try {
                    rentByLevel.push_back(stoi(token));
                } catch (...) {
                    continue;
                }
            }

            ColorGroup cg = colorGroupFromString(colorStr);
            properties.push_back(make_unique<StreetTile>(
                id, code, name, price, cg, mortgage, housePrice, hotelPrice, rentByLevel));

        } else if (type == "RAILROAD") {
            string category;
            int price, mortgage;
            iss >> category >> price >> mortgage;
            properties.push_back(make_unique<RailroadTile>(id, code, name, mortgage));

        } else if (type == "UTILITY") {
            string category;
            int price, mortgage;
            iss >> category >> price >> mortgage;
            properties.push_back(make_unique<UtilityTile>(id, code, name, mortgage));
        }
    }
}

vector<unique_ptr<PropertyTile>>& PropertyLoader::getProperties() {
    return properties;
}
