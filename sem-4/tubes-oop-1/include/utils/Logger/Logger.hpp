#pragma once
#include <string>
#include <vector>

/// @brief Represents a single event that occurred in the game log.
class LogEntry {
private:
    int turnNumber;
    std::string username;
    std::string actionType;
    std::string details;

public:
    LogEntry(int turnNumber, const std::string& username, const std::string& actionType,
             const std::string& details);

    /// @brief Gets the formatted string corresponding to the log entry specification format.
    /// @return A formatted log entry string.
    std::string toString() const;

    /// @brief Gets the formatted string equivalent for saving.
    /// @return A formatted save entry string.
    std::string toSaveString() const;

    int getTurnNumber() const;
    std::string getUsername() const;
    std::string getActionType() const;
    std::string getDetails() const;
};

/// @brief Determines the severity level of a log entry.
enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

/// @brief Records and maintains all significant transaction events automatically in a structured
/// format.
class Logger {
private:
    /// @brief Stored history of game events.
    std::vector<LogEntry> entries;

public:
    /// @brief Creates a new empty transaction log.
    Logger();
    ~Logger();

    /// @brief Appends a new event into the transaction history log.
    /// @param level The severity level of the log entry.
    /// @param turn The game cycle round when the event took place.
    /// @param player The username of the user performing the action.
    /// @param action The string identifier descriptor of the action type.
    /// @param details The narrative detailed summary of the event outcome.
    void logEvent(LogLevel level, int turn, const std::string& player, const std::string& action,
                  const std::string& details);

    /// @brief Retrieves the recent N line prints from the internal log database.
    /// @param count The number of recent elements to print. Set to -1 to view all histories.
    /// @return A collection of recent log strings describing events.
    std::vector<std::string> getRecentLogs(int count = -1) const;

    /// @brief Retrieves all entries formatting strings compatible for saving the game state.
    /// @return A vector composed of serialized logs.
    std::vector<std::string> getSaveLogs() const;

    /// @brief Clears out the transaction history cache.
    void clear();
};
