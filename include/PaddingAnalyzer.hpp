#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "RomDetector.hpp"

struct PaddingAnalysis {
    bool hasPadding = false;
    size_t trimPoint = 0;
    size_t paddingSize = 0;
    uint8_t paddingByte = 0xFF;
    double confidence = 0.0; // 0.0 a 1.0
    std::string patternType = "continuous"; // continuous, alternating, mixed
};

class PaddingAnalyzer {
public:
    PaddingAnalyzer() = default;
    ~PaddingAnalyzer() = default;
    
    PaddingAnalysis analyze(const std::string& data, uint8_t paddingByte);
    
    uint8_t autoDetectPadding(const std::string& data, RomType romType);
    
    // Métodos avançados de análise
    bool hasAlternatingPattern(const std::string& data, uint8_t paddingByte);
    bool hasMixedPadding(const std::string& data);
    double calculatePaddingConfidence(const std::string& data, 
                                     uint8_t paddingByte, 
                                     size_t paddingStart);
    
private:
    double adjustConfidenceForRomType(double baseConfidence, 
                                     size_t paddingSize, 
                                     size_t totalSize);
    
    bool validatePaddingRegion(const std::string& data, 
                              size_t start, 
                              size_t end, 
                              uint8_t paddingByte);
    
    size_t findTrueEndOfData(const std::string& data, 
                            uint8_t paddingByte,
                            size_t safetyMargin = 1024);
    
    struct PatternResult {
        bool isAlternating = false;
        bool isRepeating = false;
        size_t patternLength = 0;
    };
    
    PatternResult analyzePattern(const std::string& data, 
                                size_t start, 
                                size_t end);
};