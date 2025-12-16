#include "SafetyValidator.hpp"
#include <algorithm>

// =====================
// Validação principal
// =====================
ValidationResult SafetyValidator::validate(const std::string& data,
                                           size_t trimPoint,
                                           RomType romType,
                                           const TrimOptions& options) {
    ValidationResult result;

    validateMinimumSize(trimPoint, romType, options, result);
    validateCutRatio(data, trimPoint, options, result);
    validateSafetyMargin(trimPoint, options, result);
    validateRomSpecific(data, trimPoint, romType, result);
    validateHeaderIntegrity(data, trimPoint, romType, result);

    return result;
}

// =====================
// Regras gerais
// =====================
bool SafetyValidator::validateMinimumSize(size_t trimPoint,
                                          RomType romType,
                                          const TrimOptions&,
                                          ValidationResult& result) {
    size_t minSize = 0;

    switch (romType) {
        case RomType::GBA: minSize = MIN_GBA_SIZE; break;
        case RomType::NDS: minSize = MIN_NDS_SIZE; break;
        case RomType::GB:  minSize = MIN_GB_SIZE;  break;
        default:
            result.addError("Tipo de ROM desconhecido");
            return false;
    }

    if (trimPoint < minSize) {
        result.addError("Tamanho final abaixo do mínimo seguro");
        return false;
    }

    return true;
}

bool SafetyValidator::validateCutRatio(const std::string& data,
                                       size_t trimPoint,
                                       const TrimOptions& options,
                                       ValidationResult& result) {
    double ratio = 1.0 - (double(trimPoint) / double(data.size()));

    if (ratio > options.maxCutRatio) {
        result.addError("Corte excessivo da ROM");
        return false;
    }

    return true;
}

bool SafetyValidator::validateSafetyMargin(size_t trimPoint,
                                           const TrimOptions& options,
                                           ValidationResult& result) {
    if (trimPoint < options.safetyMargin) {
        result.addWarning("Margem de segurança muito pequena");
    }
    return true;
}

// =====================
// Validações por tipo
// =====================
bool SafetyValidator::validateRomSpecific(const std::string& data,
                                          size_t,
                                          RomType romType,
                                          ValidationResult& result) {
    bool ok = false;

    switch (romType) {
        case RomType::GBA: ok = validateGbaHeader(data); break;
        case RomType::NDS: ok = validateNdsHeader(data); break;
        case RomType::GB:  ok = validateGbHeader(data);  break;
        default: break;
    }

    if (!ok) {
        result.addError("Header da ROM inválido");
    }

    return ok;
}

bool SafetyValidator::validateHeaderIntegrity(const std::string& data,
                                              size_t,
                                              RomType romType,
                                              ValidationResult& result) {
    if (data.size() < 192) {
        result.addError("ROM pequena demais para conter header válido");
        return false;
    }

    return validateRomSpecific(data, 0, romType, result);
}

// =====================
// Headers
// =====================
bool SafetyValidator::validateGbaHeader(const std::string& data) {
    if (data.size() < 0xA0) return false;
    return calculateGbaChecksum(data) == uint8_t(data[0xBD]);
}

bool SafetyValidator::validateNdsHeader(const std::string& data) {
    if (data.size() < 512) return false;
    return readU32(data, 0x00) != 0;
}

bool SafetyValidator::validateGbHeader(const std::string& data) {
    if (data.size() < 0x150) return false;
    return calculateGbChecksum(data) == uint8_t(data[0x14D]);
}

// =====================
// Checksums
// =====================
uint8_t SafetyValidator::calculateGbaChecksum(const std::string& data) {
    uint8_t sum = 0;
    for (size_t i = 0xA0; i < 0xBD; ++i) {
        sum -= uint8_t(data[i]);
    }
    return sum - 0x19;
}

uint8_t SafetyValidator::calculateGbChecksum(const std::string& data) {
    uint8_t sum = 0;
    for (size_t i = 0x134; i <= 0x14C; ++i) {
        sum -= uint8_t(data[i]) - 1;
    }
    return sum;
}

// =====================
// Utilitário
// =====================
uint32_t SafetyValidator::readU32(const std::string& data, size_t offset) {
    if (offset + 4 > data.size()) return 0;

    return uint32_t(uint8_t(data[offset])) |
           (uint32_t(uint8_t(data[offset + 1])) << 8) |
           (uint32_t(uint8_t(data[offset + 2])) << 16) |
           (uint32_t(uint8_t(data[offset + 3])) << 24);
}

// =====================
// Análise de risco
// =====================
SafetyValidator::RiskAssessment
SafetyValidator::assessRisk(const std::string& data,
                            size_t trimPoint,
                            RomType romType) {
    RiskAssessment risk;

    double ratio = 1.0 - (double(trimPoint) / double(data.size()));
    risk.dataLossProbability = ratio;

    if (ratio < 0.2) risk.overallRisk = RiskAssessment::RiskLevel::LOW;
    else if (ratio < 0.4) risk.overallRisk = RiskAssessment::RiskLevel::MEDIUM;
    else if (ratio < 0.6) risk.overallRisk = RiskAssessment::RiskLevel::HIGH;
    else risk.overallRisk = RiskAssessment::RiskLevel::CRITICAL;

    risk.recommendation =
        (risk.overallRisk == RiskAssessment::RiskLevel::CRITICAL)
        ? "Não recomendado cortar essa ROM"
        : "Corte aceitável";

    return risk;
}