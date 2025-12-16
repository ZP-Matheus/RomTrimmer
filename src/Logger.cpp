#include "Logger.hpp"
#include "LocalizationManager.hpp"
#if defined(_WIN32)
    #include <windows.h>
#else
    #include <unistd.h>
#endif
#include <cstdio>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>

Logger::Logger() : logLevel(LogLevel::INFO), logToFile(false) {
    // Configurar nível padrão a partir de variável de ambiente
    const char* envLevel = std::getenv("ROMTRIMMER_LOG_LEVEL");
    if (envLevel) {
        std::string levelStr(envLevel);
        std::transform(levelStr.begin(), levelStr.end(), levelStr.begin(), ::toupper);
        
        if (levelStr == "DEBUG") logLevel = LogLevel::DEBUG;
        else if (levelStr == "INFO") logLevel = LogLevel::INFO;
        else if (levelStr == "WARNING") logLevel = LogLevel::WARNING;
        else if (levelStr == "ERROR") logLevel = LogLevel::ERROR;
    }
}

void Logger::log(const std::string& message, LogLevel level) {
    if (static_cast<int>(level) < static_cast<int>(logLevel)) {
        return;
    }

    std::string timestamp = getTimestamp();
    std::string levelStr = levelToString(level);

    // 🌍 Internacionalização
    std::string displayMessage = message;
    if (message.rfind("TR:", 0) == 0) { // começa com "TR:"
        std::string key = message.substr(3);
        displayMessage = LocalizationManager::instance().getString(key);
    }

    std::string formatted =
        "[" + timestamp + "] [" + levelStr + "] " + displayMessage;

    // Console
    outputToConsole(formatted, level);

    // Arquivo
    if (logToFile && logFile.is_open()) {
        logFile << formatted << std::endl;
    }

    // Buffer circular
    logBuffer.push_back(formatted);
    if (logBuffer.size() > 100) {
        logBuffer.pop_front();
    }
}

void Logger::setLogFile(const std::string& filename) {
    logFile.open(filename, std::ios::app);
    if (logFile.is_open()) {
        logToFile = true;
        log("Log iniciado em arquivo: " + filename, LogLevel::INFO);
    } else {
        log("Não pôde abrir arquivo de log: " + filename, LogLevel::ERROR);
    }
}

std::string Logger::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void Logger::outputToConsole(const std::string& message, LogLevel level) {
    // Códigos ANSI para cores (suportado na maioria dos terminais modernos)
    const std::string reset = "\033[0m";
    std::string colorCode;
    
    switch (level) {
        case LogLevel::DEBUG:
            colorCode = "\033[36m"; // Cyan
            break;
        case LogLevel::INFO:
            colorCode = "\033[32m"; // Green
            break;
        case LogLevel::WARNING:
            colorCode = "\033[33m"; // Yellow
            break;
        case LogLevel::ERROR:
            colorCode = "\033[31m"; // Red
            break;
        default:
            colorCode = reset;
            break;
    }
    
    // Verificar se é um terminal que suporta cores
    static bool isTty = (isatty(fileno(stdout)) != 0);
    
    if (isTty) {
        std::cout << colorCode << message << reset << std::endl;
    } else {
        std::cout << message << std::endl;
    }
}

std::vector<std::string> Logger::getRecentLogs(size_t count) const {
    std::vector<std::string> result;
    size_t start = (logBuffer.size() > count) ? logBuffer.size() - count : 0;
    
    for (size_t i = start; i < logBuffer.size(); ++i) {
        result.push_back(logBuffer[i]);
    }
    
    return result;
}