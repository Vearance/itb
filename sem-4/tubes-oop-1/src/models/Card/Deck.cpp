#include "Deck.hpp"
#include "Card.hpp"
#include "ChanceCard.hpp"
#include "CommunityChestCard.hpp"
#include "SkillCard.hpp"
#include <algorithm>
#include <random>

using namespace std;

template <typename T>
CardDeck<T>::CardDeck(const vector<T*>& cards) : drawPile(cards) {
    shuffle();
}

template <typename T>
CardDeck<T>::~CardDeck() {}

template <typename T>
T* CardDeck<T>::draw() {
    if (drawPile.empty()) {
        reshuffleDiscardPile();
    }
    if (drawPile.empty()) {
        return nullptr;
    }
    T* card = drawPile.back();
    drawPile.pop_back();
    return card;
}

template <typename T>
void CardDeck<T>::discard(Card* card) {
    T* typed = dynamic_cast<T*>(card);
    if (typed) {
        discardPile.push_back(typed);
    }
}

template <typename T>
void CardDeck<T>::shuffle() {
    random_device rd;
    mt19937 rng(rd());
    std::shuffle(drawPile.begin(), drawPile.end(), rng);
}

template <typename T>
void CardDeck<T>::reshuffleDiscardPile() {
    drawPile.insert(drawPile.end(), discardPile.begin(), discardPile.end());
    discardPile.clear();
    shuffle();
}

template <typename T>
bool CardDeck<T>::isEmpty() const {
    return drawPile.empty() && discardPile.empty();
}

template <typename T>
int CardDeck<T>::drawPileSize() const {
    return static_cast<int>(drawPile.size());
}

template <typename T>
int CardDeck<T>::discardPileSize() const {
    return static_cast<int>(discardPile.size());
}

template class CardDeck<ChanceCard>;
template class CardDeck<CommunityChestCard>;
template class CardDeck<SkillCard>;
