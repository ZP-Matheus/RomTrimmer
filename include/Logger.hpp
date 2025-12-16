#pragma once
#include <string>
#include <vector>
#include <deque>
#include <fstream>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    Logger();
    ~Logger() = default;

    void log(const std::string& message, LogLevel level = LogLevel::INFO);
    void setLogFile(const std::string& filename);

    std::vector<std::string> getRecentLogs(size_t count = 50) const;

private:
    LogLevel logLevel = LogLevel::INFO;
    bool logToFile = false;
    std::ofstream logFile;
    std::deque<std::string> logBuffer;

    std::string getTimestamp() const;
    std::string levelToString(LogLevel level) const;
    void outputToConsole(const std::string& message, LogLevel level);
};