#include "Board.hpp"
#include <algorithm>
#include <stdexcept>

using namespace std;

Board::Board() {}

Board::~Board() {}

Tile* Board::getTileAt(int index) const {
    if (index < 0 || index >= static_cast<int>(tiles.size())) {
        return nullptr;
    }
    return tiles[index].get();
}

Tile* Board::getTileByCode(const string& code) const {
    for (const auto& t : tiles) {
        if (t->getCode() == code) {
            return t.get();
        }
    }
    return nullptr;
}

int Board::getNewPosition(int currentPosition, int steps) const {
    int total = static_cast<int>(tiles.size());
    if (total == 0) {
        return 0;
    }
    return ((currentPosition + steps) % total + total) % total;
}

int Board::getTotalTiles() const {
    return static_cast<int>(tiles.size());
}

int Board::getJailPosition() const {
    for (int i = 0; i < static_cast<int>(tiles.size()); ++i) {
        if (tiles[i]->getCode() == "PEN") {
            return i;
        }
    }
    throw std::logic_error("Jail tile (PEN) not found on board");
}

void Board::addTile(unique_ptr<Tile> tile) {
    tiles.push_back(move(tile));
}
