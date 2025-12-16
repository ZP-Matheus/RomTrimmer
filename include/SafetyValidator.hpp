#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>
#include "RomDetector.hpp"

// =====================
// Opções de trim
// =====================
struct TrimOptions {
    double maxCutRatio = 0.6;        // Máximo de corte permitido (60%)
    size_t safetyMargin = 65536;     // Margem de segurança (64KB)
    bool strictValidation = false;   // Modo paranóico
};

// =====================
// Resultado da validação
// =====================
struct ValidationResult {
    bool valid = true;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    void addError(const std::string& msg) {
        valid = false;
        errors.push_back(msg);
    }

    void addWarning(const std::string& msg) {
        warnings.push_back(msg);
    }
};

// =====================
// SafetyValidator
// =====================
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
        std::string recommendation;
    };

    RiskAssessment assessRisk(const std::string& data,
                              size_t trimPoint,
                              RomType romType);

private:
    bool validateMinimumSize(size_t trimPoint, RomType romType,
                             const TrimOptions& options, ValidationResult& result);

    bool validateCutRatio(const std::string& data, size_t trimPoint,
                          const TrimOptions& options, ValidationResult& result);

    bool validateSafetyMargin(size_t trimPoint, const TrimOptions& options,
                              ValidationResult& result);

    bool validateRomSpecific(const std::string& data, size_t trimPoint,
                             RomType romType, ValidationResult& result);

    bool validateHeaderIntegrity(const std::string& data, size_t trimPoint,
                                 RomType romType, ValidationResult& result);

    bool validateGbaHeader(const std::string& data);
    bool validateNdsHeader(const std::string& data);
    bool validateGbHeader(const std::string& data);

    uint8_t calculateGbaChecksum(const std::string& data);
    uint8_t calculateGbChecksum(const std::string& data);

    uint32_t readU32(const std::string& data, size_t offset);

    static constexpr size_t MIN_GBA_SIZE = 1024 * 1024;
    static constexpr size_t MIN_NDS_SIZE = 8 * 1024 * 1024;
    static constexpr size_t MIN_GB_SIZE  = 32768;
};