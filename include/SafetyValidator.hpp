#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

#include "RomDetector.hpp"
#include "TrimOptions.hpp"
#include "ValidationResult.hpp"

class SafetyValidator {
public:
    ValidationResult validate(const std::string& data,
                          size_t trimPoint,
                          RomType romType,
                          const TrimOptions& options);

    bool validateGba(const std::string& data, size_t trimPoint);
    bool validateNds(const std::string& data, size_t trimPoint);
    bool validateGb(const std::string& data, size_t trimPoint);

    struct RiskAssessment {
        enum class RiskLevel { LOW, MEDIUM, HIGH, CRITICAL };
        RiskLevel overallRisk = RiskLevel::LOW;
        double dataLossProbability = 0.0;
        std::vector<std::string> riskFactors;
    };

    RiskAssessment assessRisk(const std::string& data,
                              size_t trimPoint,
                              RomType romType);

private:
    size_t getMinSizeForRomType(RomType type);
    size_t getRecommendedSizeForRomType(RomType type);

    bool validateGbaInternalRomSize(const std::string& data, size_t trimPoint);
    bool validateNdsSectionOffsets(const std::string& data, size_t trimPoint);
    bool validateGbRomSize(size_t size);

    bool validateKnownStructuresInternal(const std::string& data,
                                         size_t trimPoint,
                                         RomType romType);

    uint32_t readU32(const std::string& data, size_t offset);

    static constexpr size_t MIN_GBA_SIZE = 1024 * 1024;
    static constexpr size_t MIN_NDS_SIZE = 8 * 1024 * 1024;
    static constexpr size_t MIN_GB_SIZE  = 32768;
};