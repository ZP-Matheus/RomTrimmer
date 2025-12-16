#include "SafetyValidator.hpp"
#include "ValidationResult.hpp"
#include "TrimOptions.hpp"
#include "Localization.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstring>

// ==================== DEFINIÇÃO DA STRUCT TRIMOPTIONS ====================
#ifndef TRIM_OPTIONS_DEFINED
#define TRIM_OPTIONS_DEFINED

#endif

ValidationResult SafetyValidator::validate(
    const std::string& data,
    size_t trimPoint,
    RomType romType,
    const TrimOptions& options) {
    
    ValidationResult result;
    result.isValid = true;
    result.message = "OK";
    
    try {
        // 1. Validar tamanho mínimo absoluto
        size_t minSizeByType = getMinSizeForRomType(romType);
        size_t minSize = std::max(options.minSize, minSizeByType);
        
        if (trimPoint < minSize) {
            result.isValid = false;
            result.message = TR("FINAL_SIZE_BELOW_MIN") + std::to_string(trimPoint) + 
                            TR("BELOW_MINIMUM_ALLOWED") + std::to_string(minSize) + ")";
            return result;
        }
        
        // 2. Validar porcentagem máxima de corte
        double cutRatio = 1.0 - (static_cast<double>(trimPoint) / data.size());
        if (cutRatio > options.maxCutRatio) {
            result.isValid = false;
            result.message = TR("CUT_TOO_AGGRESSIVE") + 
                            std::to_string(static_cast<int>(cutRatio * 100)) + 
                            TR("EXCEEDS_LIMIT") + 
                            std::to_string(static_cast<int>(options.maxCutRatio * 100)) + "%)";
            return result;
        }
        
        // 3. Validar margem de segurança
        if (trimPoint < options.safetyMargin) {
            result.isValid = false;
            result.message = TR("FILE_TOO_SMALL_AFTER_TRIM") + 
                            std::to_string(trimPoint) + 
                            TR("BELOW_SAFETY_MARGIN");
            return result;
        }
        
        // 4. Validações específicas por tipo de ROM
        switch (romType) {
            case RomType::GBA:
                if (!validateGba(data, trimPoint)) {
                    result.isValid = false;
                    result.message = TR("GBA_VALIDATION_FAILED");
                }
                break;
                
            case RomType::NDS:
                if (!validateNds(data, trimPoint)) {
                    result.isValid = false;
                    result.message = TR("NDS_VALIDATION_FAILED");
                }
                break;
                
            case RomType::GB:
                if (!validateGb(data, trimPoint)) {
                    result.isValid = false;
                    result.message = TR("GB_VALIDATION_FAILED");
                }
                break;
                
            default:
                // Para tipos desconhecidos, ser mais conservador
                if (cutRatio > 0.1) {
                    result.isValid = false;
                    result.message = TR("CUT_TOO_LARGE_UNKNOWN_ROM");
                }
                break;
        }
        
        // 5. Verificar se o ponto de corte não interrompe estruturas conhecidas
        if (!validateKnownStructuresInternal(data, trimPoint, romType)) {
            result.isValid = false;
            result.message = TR("CUT_INTERRUPTS_KNOWN_STRUCTURES");
        }
        
    } catch (const std::exception& e) {
        result.isValid = false;
        result.message = "Exception during validation: " + std::string(e.what());
    }
    
    return result;
}

size_t SafetyValidator::getMinSizeForRomType(RomType type) {
    switch (type) {
        case RomType::GBA:
            return MIN_GBA_SIZE;
        case RomType::NDS:
            return MIN_NDS_SIZE;
        case RomType::GB:
            return MIN_GB_SIZE;
        default:
            return 1024;
    }
}

bool SafetyValidator::validateGba(const std::string& data, size_t trimPoint) {
    // Verificar se não estamos cortando após o logo Nintendo
    if (trimPoint < 0xA0) {
        return false;
    }
    
    // Verificar se o tamanho final faz sentido para GBA
    if (trimPoint > 32 * 1024 * 1024) {
        return false;
    }
    
    return validateGbaInternalRomSize(data, trimPoint);
}

bool SafetyValidator::validateNds(const std::string& data, size_t trimPoint) {
    if (data.size() < 512) {
        return false;
    }
    
    return validateNdsSectionOffsets(data, trimPoint);
}

bool SafetyValidator::validateGb(const std::string& data, size_t trimPoint) {
    return validateGbRomSize(trimPoint);
}

bool SafetyValidator::validateGbaInternalRomSize(const std::string& data, size_t trimPoint) {
    // Verificar alinhamento comum para GBA
    if (trimPoint % 0x1000 == 0) { // 4KB alinhamento comum
        return true;
    }
    
    // Se não for alinhado, verificar se há dados não-padding
    const size_t checkWindow = 1024;
    size_t checkStart = trimPoint;
    size_t checkEnd = std::min(data.size(), trimPoint + checkWindow);
    
    for (size_t i = checkStart; i < checkEnd; ++i) {
        if (data[i] != 0xFF && data[i] != 0x00) {
            return false;
        }
    }
    
    return true;
}

bool SafetyValidator::validateNdsSectionOffsets(const std::string& data, size_t trimPoint) {
    uint32_t arm9Offset = readU32(data, 0x20);
    uint32_t arm9Size = readU32(data, 0x2C);
    
    uint32_t arm7Offset = readU32(data, 0x30);
    uint32_t arm7Size = readU32(data, 0x3C);
    
    // Verificar se não estamos cortando dentro das seções
    if (trimPoint > arm9Offset && trimPoint < arm9Offset + arm9Size) {
        return false;
    }
    
    if (trimPoint > arm7Offset && trimPoint < arm7Offset + arm7Size) {
        return false;
    }
    
    // Verificar alinhamento comum para NDS
    if (trimPoint % 0x200 != 0) {
        return false;
    }
    
    return true;
}

bool SafetyValidator::validateGbRomSize(size_t size) {
    const size_t validSizes[] = {
        32768,   65536,   131072,  262144,
        524288,  1048576, 2097152, 4194304, 8388608
    };
    
    for (size_t validSize : validSizes) {
        if (size == validSize) {
            return true;
        }
        
        // Permitir pequena tolerância
        if (std::abs(static_cast<long>(size) - static_cast<long>(validSize)) < 
            static_cast<long>(validSize * 0.01)) {
            return true;
        }
    }
    
    return false;
}

bool SafetyValidator::validateKnownStructuresInternal(const std::string& data, 
                                                    size_t trimPoint, 
                                                    RomType romType) {
    if (trimPoint < data.size() && trimPoint > 0) {
        // Verificar strings comuns
        const char* commonStrings[] = {
            "Nintendo", "LICENSED", "LICENSED BY", 
            "PUBLISHER", "DEVELOPER", "TM", "R"
        };
        
        for (const char* str : commonStrings) {
            size_t len = strlen(str);
            if (trimPoint >= len) {
                for (size_t i = trimPoint - len; i < trimPoint && i + len < data.size(); ++i) {
                    bool match = true;
                    for (size_t j = 0; j < len; ++j) {
                        if (data[i + j] != str[j]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
}

SafetyValidator::RiskAssessment SafetyValidator::assessRisk(
    const std::string& data,
    size_t trimPoint,
    RomType romType) {
    
    RiskAssessment assessment;
    assessment.overallRisk = RiskAssessment::RiskLevel::LOW;
    
    if (data.empty()) {
        assessment.overallRisk = RiskAssessment::RiskLevel::CRITICAL;
        assessment.riskFactors.push_back("Empty data");
        return assessment;
    }
    
    double cutRatio = 1.0 - (static_cast<double>(trimPoint) / data.size());
    assessment.dataLossProbability = std::min(cutRatio * 1.5, 1.0);
    
    if (cutRatio > 0.5) {
        assessment.riskFactors.push_back("Large cut detected");
        assessment.overallRisk = RiskAssessment::RiskLevel::HIGH;
    }
    
    if (trimPoint < getRecommendedSizeForRomType(romType)) {
        assessment.riskFactors.push_back("Below recommended size");
        assessment.overallRisk = std::max(assessment.overallRisk, 
                                         RiskAssessment::RiskLevel::MEDIUM);
    }
    
    if (!validateKnownStructuresInternal(data, trimPoint, romType)) {
        assessment.riskFactors.push_back("Structure conflict detected");
        assessment.overallRisk = RiskAssessment::RiskLevel::HIGH;
    }
    
    return assessment;
}

size_t SafetyValidator::getRecommendedSizeForRomType(RomType type) {
    switch (type) {
        case RomType::GBA: return 8 * 1024 * 1024;
        case RomType::NDS: return 64 * 1024 * 1024;
        case RomType::GB: return 524288;
        default: return 8192;
    }
}

uint32_t SafetyValidator::readU32(const std::string& data, size_t offset) {
    if (offset + 4 > data.size()) {
        return 0;
    }
    
    return static_cast<uint8_t>(data[offset]) |
           (static_cast<uint8_t>(data[offset + 1]) << 8) |
           (static_cast<uint8_t>(data[offset + 2]) << 16) |
           (static_cast<uint8_t>(data[offset + 3]) << 24);
}

// Implementações vazias para funções não usadas
bool SafetyValidator::validateMinimumSize(size_t, RomType, const TrimOptions&, ValidationResult&) { return true; }
bool SafetyValidator::validateCutRatio(const std::string&, size_t, const TrimOptions&, ValidationResult&) { return true; }
bool SafetyValidator::validateSafetyMargin(size_t, const TrimOptions&, ValidationResult&) { return true; }
bool SafetyValidator::validateRomSpecific(const std::string&, size_t, RomType, ValidationResult&) { return true; }
bool SafetyValidator::validateKnownStructures(const std::string&, size_t, RomType, ValidationResult&) { return true; }
bool SafetyValidator::validateHeaderIntegrity(const std::string&, size_t, RomType, ValidationResult&) { return true; }
std::string SafetyValidator::analyzeStructureConflict(const std::string&, size_t, RomType) { return ""; }
bool SafetyValidator::validateHeaderIntegrityInternal(const std::string&, size_t, RomType) { return true; }
bool SafetyValidator::validateGbaHeader(const std::string&, size_t) { return true; }
bool SafetyValidator::validateNdsHeader(const std::string&, size_t) { return true; }
bool SafetyValidator::validateGbHeader(const std::string&, size_t) { return true; }
uint8_t SafetyValidator::calculateGbaChecksum(const std::string&) { return 0; }
uint8_t SafetyValidator::calculateGbChecksum(const std::string&) { return 0; }
bool SafetyValidator::containsKnownDataPatterns(const std::string&, size_t, size_t) { return false; }