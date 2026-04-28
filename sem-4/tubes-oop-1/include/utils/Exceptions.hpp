#pragma once
#include <exception>
#include <string>

/// @brief Custom exception classes for handling various error scenarios in the Nimonspoli game.
class GameException : public std::exception {
protected:
    std::string message;

public:
    GameException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

/// @brief Exception thrown when a player attempts to perform an action without sufficient funds.
class InsufficientFundsException : public GameException {
public:
    InsufficientFundsException(int required, int available);
};

/// @brief Exception thrown when a player tries to purchase a property that is not available for
/// sale.
class InvalidPropertyException : public GameException {
public:
    InvalidPropertyException(const std::string& reason);
};

/// @brief Exception thrown when a player tries to use a card that has reached its usage limit.
class CardLimitException : public GameException {
public:
    CardLimitException();
};

/// @brief Exception thrown when a player attempts to perform an action that is not valid in the
/// current game state.
class InvalidCommandException : public GameException {
public:
    InvalidCommandException(const std::string& command);
};

/// @brief Exception thrown when an error occurs during file operations, such as loading or saving
/// game data.
class FileException : public GameException {
public:
    FileException(const std::string& filename, const std::string& operation);
};

/// @brief Exception thrown when a player tries to perform an action that is not valid in the
/// current game state.
class GameStateException : public GameException {
public:
    GameStateException(const std::string& reason);
};
