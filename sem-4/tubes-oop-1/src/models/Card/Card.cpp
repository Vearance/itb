#include "Card.hpp"

using namespace std;

static int nextCardId = 1;

Card::Card(const string& name, const string& description)
    : id(nextCardId++), name(name), description(description) {}

Card::~Card() {}

int Card::getId() const {
    return id;
}

const string& Card::getName() const {
    return name;
}

const string& Card::getDescription() const {
    return description;
}
