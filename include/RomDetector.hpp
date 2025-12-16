#pragma once
#include <string>
#include <cstdint>

enum class RomType {
    UNKNOWN,
    GBA,
    NDS,
    GB,
    GBC
};

class RomDetector {
public:
    RomDetector() = default;
    ~RomDetector() = default;
    
    RomType detect(const std::string& data);
    
private:
    bool isGbaRom(const std::string& data);
    bool isNdsRom(const std::string& data);
    bool isGbRom(const std::string& data);
    
    size_t findLastNonPadding(const std::string& data, uint8_t padding);
    bool isPowerOfTwo(size_t n);
};