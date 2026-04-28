#include "Logger.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace std;

LogEntry::LogEntry(int turnNumber, const string& username, const string& actionType,
                   const string& details)
    : turnNumber(turnNumber), username(username), actionType(actionType), details(details) {}

string LogEntry::toString() const {
    ostringstream oss;
    oss << "[Turn " << turnNumber << "] " << username << " | " 
        << left << setw(8) << actionType << " | " << details;
    return oss.str();
}

string LogEntry::toSaveString() const {
    ostringstream oss;
    oss << turnNumber << " " << username << " " << actionType << " " << details;
    return oss.str();
}

int LogEntry::getTurnNumber() const {
    return turnNumber;
}

string LogEntry::getUsername() const {
    return username;
}

string LogEntry::getActionType() const {
    return actionType;
}

string LogEntry::getDetails() const {
    return details;
}

Logger::Logger() {}

Logger::~Logger() {}

void Logger::logEvent(LogLevel level, int turn, const string& player, const string& action,
                      const string& details) {
    entries.emplace_back(turn, player, action, details);
}

vector<string> Logger::getRecentLogs(int count) const {
    vector<string> result;

    if (count == -1) {
        for (const auto& entry : entries) {
            result.push_back(entry.toString());
        }
    } else {
        int start = max(0, static_cast<int>(entries.size()) - count);
        for (int i = start; i < static_cast<int>(entries.size()); ++i) {
            result.push_back(entries[i].toString());
        }
    }

    return result;
}

vector<string> Logger::getSaveLogs() const {
    vector<string> result;
    for (const auto& entry : entries) {
        result.push_back(entry.toSaveString());
    }
    return result;
}

void Logger::clear() {
    entries.clear();
}
