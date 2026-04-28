#include "TaxLoader.hpp"
#include <fstream>
#include <stdexcept>

using namespace std;

TaxLoader::TaxLoader(const string& file) : Loader(file) {}

TaxLoader::~TaxLoader() {}

void TaxLoader::loadConfig() {
    ifstream in(filename);
    if (!in) throw runtime_error("Cannot open " + filename);
    in >> pphFlat >> pphPercentage >> pbmFlat;
}

int TaxLoader::getPphFlat() const {
    return pphFlat;
}

int TaxLoader::getPphPercentage() const {
    return pphPercentage;
}

int TaxLoader::getPbmFlat() const {
    return pbmFlat;
}
