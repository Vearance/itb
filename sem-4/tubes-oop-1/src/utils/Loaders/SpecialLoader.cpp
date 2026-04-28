#include "SpecialLoader.hpp"
#include <fstream>
#include <stdexcept>

using namespace std;

SpecialLoader::SpecialLoader(const string& file) : Loader(file) {}

SpecialLoader::~SpecialLoader() {}

void SpecialLoader::loadConfig() {
    ifstream in(filename);
    if (!in) throw runtime_error("Cannot open " + filename);
    in >> goSalary >> jailFine;
}

int SpecialLoader::getGoSalary() const {
    return goSalary;
}

int SpecialLoader::getJailFine() const {
    return jailFine;
}
