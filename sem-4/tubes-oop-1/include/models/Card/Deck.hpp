#pragma once
#include <vector>

class Card;

/**
 * @brief Non-templated deck interface — lets CardTile hold any deck without
 * coupling to a specific card type.
 */
class ICardDeck {
public:
    virtual ~ICardDeck() = default;
    virtual Card* draw() = 0;
    virtual void discard(Card* card) = 0;
    virtual bool isEmpty() const = 0;
    virtual void reshuffleDiscardPile() = 0;
    virtual int drawPileSize() const = 0;
    virtual int discardPileSize() const = 0;
};

/**
 * @brief Type-safe generic deck. T must publicly derive from Card.
 * @tparam T Concrete card type stored in this deck.
 */
template <typename T>
class CardDeck : public ICardDeck {
private:
    std::vector<T*> drawPile;
    std::vector<T*> discardPile;

public:
    explicit CardDeck(const std::vector<T*>& cards);
    ~CardDeck() override;

    T* draw() override;
    void discard(Card* card) override;
    void shuffle();
    void reshuffleDiscardPile() override;

    bool isEmpty() const override;
    int drawPileSize() const override;
    int discardPileSize() const override;
};
