#include "Tile.hpp"

Tile::Tile(const int id, const std::string& code, const std::string& name)
    : id(id), code(code), name(name) {}

Tile::~Tile() = default;

int Tile::getId() const {
    return id;
}

const std::string& Tile::getCode() const {
    return code;
}

const std::string& Tile::getName() const {
    return name;
}
