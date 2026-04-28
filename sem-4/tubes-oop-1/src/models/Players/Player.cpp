#include "Player.hpp"
#include "Exceptions.hpp"
#include "PropertyTile.hpp"
#include "SkillCard.hpp"
#include "StreetTile.hpp"
#include <algorithm>

using namespace std;

// ── Constructor / Destructor ──────────────────────────────────────────────────

Player::Player(int id, const string& username)
    : id(id), username(username), money(0), status(PlayerStatus::ACTIVE), position(0), jailTurns(0),
      isComputer(false), hasUsedSkillThisTurn(false), hasRolledDice(false), consecutiveDoubles(0),
      discountPercentage(0), discountTurnsLeft(0), shieldTurnsLeft(0), getOutOfJailCard(false) {}

Player::~Player() {}

// ── Operator overloads ────────────────────────────────────────────────────────

Player& Player::operator+=(int amount) {
    money += amount;
    return *this;
}

Player& Player::operator-=(int amount) {
    money -= amount;
    return *this;
}

bool Player::operator<(const Player& other) const {
    if (money != other.money)
        return money < other.money;
    if (properties.size() != other.properties.size())
        return properties.size() < other.properties.size();
    return hand.size() < other.hand.size();
}

bool Player::operator>(const Player& other) const {
    return other < *this;
}

// ── Basic getters ─────────────────────────────────────────────────────────────

int Player::getId() const {
    return id;
}

const string& Player::getUsername() const {
    return username;
}

int Player::getMoney() const {
    return money;
}

int Player::getPosition() const {
    return position;
}

PlayerStatus Player::getStatus() const {
    return status;
}

int Player::getJailTurns() const {
    return jailTurns;
}

int Player::getConsecutiveDoubles() const {
    return consecutiveDoubles;
}

const vector<SkillCard*>& Player::getHand() const {
    return hand;
}

const vector<PropertyTile*>& Player::getProperties() const {
    return properties;
}

bool Player::getIsComputer() const {
    return isComputer;
}

void Player::setIsComputer(bool value) {
    isComputer = value;
}

// ── Wealth helpers ────────────────────────────────────────────────────────────

int Player::getLiquidWealth() const {
    return money;
}

int Player::getPropertyValue() const {
    int total = 0;
    for (const auto* prop : properties) {
        total += prop->getPrice();
    }
    return total;
}

int Player::getTotalWealth() const {
    int total = money + getPropertyValue();
    for (const auto* prop : properties) {
        const auto* street = dynamic_cast<const StreetTile*>(prop);
        if (!street)
            continue;
        int level = street->getPropertyLevel();
        if (level >= 1 && level <= 4) {
            total += level * street->getHousePrice();
        } else if (level == 5) {
            total += 4 * street->getHousePrice() + street->getHotelPrice();
        }
    }
    return total;
}

// ── Setters ───────────────────────────────────────────────────────────────────

void Player::setPosition(int pos) {
    position = pos;
}

void Player::setStatus(PlayerStatus newStatus) {
    status = newStatus;
}

void Player::setJailTurns(int turns) {
    jailTurns = turns;
}

void Player::incrementJailTurns() {
    ++jailTurns;
}

void Player::resetJailTurns() {
    jailTurns = 0;
}

// ── Turn-state flags ──────────────────────────────────────────────────────────

bool Player::getHasUsedSkillThisTurn() const {
    return hasUsedSkillThisTurn;
}

void Player::setUsedSkillThisTurn(bool value) {
    hasUsedSkillThisTurn = value;
}

bool Player::getHasRolledDice() const {
    return hasRolledDice;
}

void Player::setRolledDice(bool value) {
    hasRolledDice = value;
}

void Player::resetTurnFlags() {
    hasUsedSkillThisTurn = false;
    hasRolledDice = false;
    consecutiveDoubles = 0;
}

// ── Jail helpers ──────────────────────────────────────────────────────────────

bool Player::isInJail() const {
    return status == PlayerStatus::JAILED;
}

void Player::goToJail() {
    status = PlayerStatus::JAILED;
    jailTurns = 0;
}

void Player::releaseFromJail() {
    status = PlayerStatus::ACTIVE;
    jailTurns = 0;
}

// ── Property management ───────────────────────────────────────────────────────

void Player::addProperty(PropertyTile* property) {
    if (!property)
        return;
    if (find(properties.begin(), properties.end(), property) == properties.end()) {
        properties.push_back(property);
    }
}

void Player::removeProperty(PropertyTile* property) {
    properties.erase(remove(properties.begin(), properties.end(), property), properties.end());
}

bool Player::hasMonopoly(ColorGroup group) const {
    for (const auto* prop : properties) {
        const auto* street = dynamic_cast<const StreetTile*>(prop);
        if (street && street->getColorGroup() == group && street->isMonopolyOwned()) {
            return true;
        }
    }
    return false;
}

vector<ColorGroup> Player::getMonopolyGroups() const {
    vector<ColorGroup> monopolies;
    for (const auto* prop : properties) {
        const auto* street = dynamic_cast<const StreetTile*>(prop);
        if (!street || !street->isMonopolyOwned())
            continue;
        ColorGroup g = street->getColorGroup();
        if (find(monopolies.begin(), monopolies.end(), g) == monopolies.end()) {
            monopolies.push_back(g);
        }
    }
    return monopolies;
}

// ── Skill card management ─────────────────────────────────────────────────────

void Player::addCard(SkillCard* card) {
    if (hand.size() >= 3) {
        throw CardLimitException();
    }
    hand.push_back(card);
}

void Player::removeCard(SkillCard* card) {
    hand.erase(remove(hand.begin(), hand.end(), card), hand.end());
}

bool Player::hasCard(SkillCard* card) const {
    return find(hand.begin(), hand.end(), card) != hand.end();
}

// ── Active card effects ───────────────────────────────────────────────────────

void Player::applyDiscount(int percentage, int turns) {
    discountPercentage = percentage;
    discountTurnsLeft = turns;
}

void Player::tickDiscount() {
    if (discountTurnsLeft > 0 && --discountTurnsLeft == 0) {
        discountPercentage = 0;
    }
}

bool Player::hasActiveDiscount() const {
    return discountTurnsLeft > 0;
}

int Player::getDiscountPercentage() const {
    return discountPercentage;
}

void Player::activateShield(int turns) {
    shieldTurnsLeft = turns;
}

void Player::tickShield() {
    if (shieldTurnsLeft > 0) {
        --shieldTurnsLeft;
    }
}

bool Player::isShielded() const {
    return shieldTurnsLeft > 0;
}

bool Player::hasJailFreeCard() const {
    return getOutOfJailCard;
}

void Player::setJailFreeCard(bool value) {
    getOutOfJailCard = value;
}

void Player::useJailFreeCard() {
    getOutOfJailCard = false;
}
