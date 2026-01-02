// Ucon64Integration.hpp
#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <optional>

class Ucon64Integration {
public:
    struct Ucon64Result {
        bool isPadded;
        uint64_t paddedBytes;
        std::string consoleType;
        std::string ucon64Output;
    };
    
    static std::optional<Ucon64Result> analyzeWithUcon64(const std::string& filePath);
    static std::optional<uint64_t> getPaddingBytes(const std::string& filePath);
    
private:
    static std::string executeUcon64Command(const std::string& command);
    static std::string detectConsoleForUcon64(RomType romType);
};
