// Ucon64Integration.cpp
#include "Ucon64Integration.hpp"
#include "RomDetector.hpp"
#include <fstream>
#include <sstream>
#include <regex>
#include <cstdlib>

std::optional<Ucon64Integration::Ucon64Result> 
Ucon64Integration::analyzeWithUcon64(const std::string& filePath) {
    // Try to find ucon64 executable
    std::string ucon64Path;
    
    #ifdef _WIN32
    ucon64Path = "ucon64.exe";
    #else
    ucon64Path = "ucon64";
    #endif
    
    // Check if ucon64 exists and is executable
    if (system((ucon64Path + " --version > nul 2>&1").c_str()) != 0) {
        return std::nullopt; // ucon64 not found
    }
    
    // First detect ROM type to get console flag
    std::ifstream file(filePath, std::ios::binary);
    if (!file) return std::nullopt;
    
    std::string data((std::istreambuf_iterator<char>(file)), 
                     std::istreambuf_iterator<char>());
    
    RomDetector detector;
    RomType romType = detector.detect(data);
    
    std::string consoleFlag = detectConsoleForUcon64(romType);
    std::string command = ucon64Path + " ";
    
    if (!consoleFlag.empty()) {
        command += consoleFlag + " ";
    }
    
    command += "--ispad \"" + filePath + "\"";
    
    std::string output = executeUcon64Command(command);
    
    // Parse ucon64 output
    Ucon64Result result;
    result.ucon64Output = output;
    
    // Check for padding info in output
    std::regex padRegex(R"(Padded\s+(\d+)\s+bytes)");
    std::smatch matches;
    
    if (std::regex_search(output, matches, padRegex)) {
        result.isPadded = true;
        result.paddedBytes = std::stoull(matches[1].str());
    } else {
        result.isPadded = false;
        result.paddedBytes = 0;
    }
    
    // Extract console type if mentioned
    std::regex consoleRegex(R"(as\s+([A-Za-z0-9]+))");
    if (std::regex_search(output, matches, consoleRegex)) {
        result.consoleType = matches[1].str();
    }
    
    return result;
}

std::optional<uint64_t> Ucon64Integration::getPaddingBytes(const std::string& filePath) {
    auto result = analyzeWithUcon64(filePath);
    if (result) {
        return result->paddedBytes;
    }
    return std::nullopt;
}

std::string Ucon64Integration::executeUcon64Command(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    
    #ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
    #else
    FILE* pipe = popen(command.c_str(), "r");
    #endif
    
    if (!pipe) return "";
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    #ifdef _WIN32
    _pclose(pipe);
    #else
    pclose(pipe);
    #endif
    
    return result;
}

std::string Ucon64Integration::detectConsoleForUcon64(RomType romType) {
    switch (romType) {
        case RomType::GBA: return "--gba";
        case RomType::NDS: return "--nds";
        case RomType::GB:  return "--gb";
        case RomType::GBC: return "--gbc";
        default: return "";
    }
}
