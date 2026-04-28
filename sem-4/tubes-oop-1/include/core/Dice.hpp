#pragma once

#include <random>
#include <string>
#include <vector>

/// @brief Simulates throwing two 6-sided dice for the game.
class Dice {
private:
    /// @brief Result of the first die.
    int die1;

    /// @brief Result of the second die.
    int die2;

    /// @brief Counter tracking how many consecutive times doubles have been rolled.
    int consecutiveDoubles;

    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;

public:
    /// @brief Creates a dice object with an initial neutral state.
    Dice();
    ~Dice();

    /// @brief Randomizes the dice using an RNG, updating the die values, checking doubles.
    /// @return Total value of the rolled dice.
    int rollRandom();

    /// @brief Sets the dice results manually by the user.
    /// @param d1 Result of the first die.
    /// @param d2 Result of the second die.
    /// @return Total value of the updated dice.
    int rollManual(int d1, int d2);

    /// @brief Gets the total accumulated result from the latest throw.
    /// @return Current total sum of die1 and die2.
    int getTotal() const;

    /// @brief Determines if the latest roll was a double (both die numbers are equal).
    /// @return True if it was a double, false otherwise.
    bool isDouble() const;

    /// @brief Determines if the player has exceeded the speeding limit (throw doubles 3 times in a row).
    /// @return True if the roll generated a speed violation (3 doubles).
    bool isSpeedingViolation() const;

    /// @brief Resets the continuous double rolls counter (for example, end of player's turn).
    void resetDoublesCount();

    /// @brief Retrieves the individual results of the dice.
    /// @return A pair of integers containing the values of the two dice.
    std::pair<int, int> getDiceValues() const;
};
