#include "TurnManager.hpp"
#include "Player.hpp"
#include <algorithm>
#include <random>

using namespace std;

TurnManager::TurnManager(int maxTurn) : activeIndex(0), currentTurn(1), maxTurn(maxTurn) {}

TurnManager::~TurnManager() {}

void TurnManager::initOrder(int playerCount) {
    turnOrder.resize(playerCount);
    for (int i = 0; i < playerCount; ++i) turnOrder[i] = i;
    random_device rd;
    mt19937 rng(rd());
    shuffle(turnOrder.begin(), turnOrder.end(), rng);
    activeIndex = 0;
    currentTurn = 1;
}

void TurnManager::loadOrder(const vector<int>& order, int index, int turn) {
    turnOrder = order;
    activeIndex = index;
    currentTurn = turn;
}

void TurnManager::advance(const vector<Player*>& /*activePlayers*/) {
    if (turnOrder.empty()) return;
    activeIndex = (activeIndex + 1) % static_cast<int>(turnOrder.size());
    if (activeIndex == 0) {
        ++currentTurn;
    }
}

void TurnManager::removePlayer(int playerIndex) {
    auto it = find(turnOrder.begin(), turnOrder.end(), playerIndex);
    if (it == turnOrder.end()) return;

    int removedPos = static_cast<int>(it - turnOrder.begin());
    turnOrder.erase(it);

    if (turnOrder.empty()) return;
    if (removedPos < activeIndex) {
        --activeIndex;
    }
    activeIndex = activeIndex % static_cast<int>(turnOrder.size());
}

int TurnManager::getActivePlayerIndex() const {
    if (turnOrder.empty()) return -1;
    return turnOrder[activeIndex];
}

int TurnManager::getCurrentTurn() const {
    return currentTurn;
}

int TurnManager::getMaxTurn() const {
    return maxTurn;
}

const vector<int>& TurnManager::getTurnOrder() const {
    return turnOrder;
}

bool TurnManager::hasReachedMaxTurn() const {
    return maxTurn > 0 && currentTurn > maxTurn;
}
