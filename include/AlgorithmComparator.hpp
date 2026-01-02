// AlgorithmComparator.hpp
#pragma once
#include <string>
#include <vector>
#include <map>

struct ComparisonResult {
    std::string filename;
    size_t ourPaddingBytes;
    size_t ucon64PaddingBytes;
    bool match;
    double differencePercentage;
    std::string notes;
};

class AlgorithmComparator {
public:
    static std::vector<ComparisonResult> compareWithUcon64(
        const std::vector<std::string>& files);
    
    static void generateComparisonReport(
        const std::vector<ComparisonResult>& results,
        const std::string& outputFile);
    
private:
    static size_t calculatePaddingUsingOurAlgorithm(const std::string& filePath);
};
