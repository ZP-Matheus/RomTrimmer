#include "SafetyValidator.hpp"
#include "ValidationResult.hpp"
#include "TrimOptions.hpp"
#include "Localization.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <sstream>
#include <iomanip>

// ==================== TAMANHOS ====================

size_t SafetyValidator::getMinSizeForRomType(RomType type) {
    switch (type) {
        case RomType::GBA: return MIN_GBA_SIZE;
        case RomType::NDS: return MIN_NDS_SIZE;
        case RomType::GB:  return MIN_GB_SIZE;
        default:           return 1024;
    }
}

// ==================== GBA ====================

bool SafetyValidator::validateGba(const std::string& data, size_t trimPoint) {
    if (trimPoint < 0xA0) return false;
    if (trimPoint > 32 * 1024 * 1024) return false;
    return validateGbaInternalRomSize(data, trimPoint);
}

bool SafetyValidator::validateGbaInternalRomSize(
    const std::string& data, size_t trimPoint)
{
    if (trimPoint % 0x1000 == 0) return true;

    const size_t checkWindow = 1024;
    size_t end = std::min(data.size(), trimPoint + checkWindow);

    for (size_t i = trimPoint; i < end; ++i) {
        if (data[i] != '\0' && data[i] != '\xFF') {
            return false;
        }
    }
    return true;
}

// ==================== NDS ====================

bool SafetyValidator::validateNds(const std::string& data, size_t trimPoint) {
    if (data.size() < 512) return false;
    return validateNdsSectionOffsets(data, trimPoint);
}

bool SafetyValidator::validateNdsSectionOffsets(
    const std::string& data, size_t trimPoint)
{
    uint32_t arm9Offset = readU32(data, 0x20);
    uint32_t arm9Size   = readU32(data, 0x2C);
    uint32_t arm7Offset = readU32(data, 0x30);
    uint32_t arm7Size   = readU32(data, 0x3C);

    if (trimPoint > arm9Offset && trimPoint < arm9Offset + arm9Size) return false;
    if (trimPoint > arm7Offset && trimPoint < arm7Offset + arm7Size) return false;

    return (trimPoint % 0x200) == 0;
}

// ==================== GB ====================

bool SafetyValidator::validateGb(const std::string&, size_t trimPoint) {
    return validateGbRomSize(trimPoint);
}

bool SafetyValidator::validateGbRomSize(size_t size) {
    const size_t validSizes[] = {
        32768, 65536, 131072, 262144,
        524288, 1048576, 2097152,
        4194304, 8388608
    };

    for (size_t v : validSizes) {
        if (size == v) return true;
        if (std::abs((long)size - (long)v) < (long)(v * 0.01))
            return true;
    }
    return false;
}

// ==================== ESTRUTURAS CONHECIDAS ====================

bool SafetyValidator::validateKnownStructuresInternal(
    const std::string& data,
    size_t trimPoint,
    RomType)
{
    static const char* commonStrings[] = {
        "Nintendo", "GAME BOY", "POKEMON", "SEGA"
    };

    if (trimPoint > data.size() || trimPoint < 32)
        return true;

    for (const char* str : commonStrings) {
        size_t len = std::strlen(str);
        if (trimPoint < len) continue;

        size_t start = trimPoint - len;
        if (start + len > data.size()) continue;

        if (std::memcmp(data.data() + start, str, len) == 0)
            return false;
    }
    return true;
}

// ==================== RISCO ====================

SafetyValidator::RiskAssessment SafetyValidator::assessRisk(
    const std::string& data,
    size_t trimPoint,
    RomType romType)
{
    RiskAssessment r;
    r.overallRisk = RiskAssessment::RiskLevel::LOW;

    if (data.empty()) {
        r.overallRisk = RiskAssessment::RiskLevel::CRITICAL;
        r.riskFactors.push_back("Empty data");
        return r;
    }

    double cutRatio = 1.0 - (double(trimPoint) / data.size());
    r.dataLossProbability = std::min(cutRatio * 1.5, 1.0);

    if (cutRatio > 0.5) {
        r.overallRisk = RiskAssessment::RiskLevel::HIGH;
        r.riskFactors.push_back("Large cut detected");
    }

    if (trimPoint < getRecommendedSizeForRomType(romType)) {
        r.overallRisk =
            std::max(r.overallRisk, RiskAssessment::RiskLevel::MEDIUM);
        r.riskFactors.push_back("Below recommended size");
    }

    if (!validateKnownStructuresInternal(data, trimPoint, romType)) {
        r.overallRisk = RiskAssessment::RiskLevel::HIGH;
        r.riskFactors.push_back("Structure conflict detected");
    }

    return r;
}

// ==================== UTIL ====================

size_t SafetyValidator::getRecommendedSizeForRomType(RomType type) {
    switch (type) {
        case RomType::GBA: return 8 * 1024 * 1024;
        case RomType::NDS: return 64 * 1024 * 1024;
        case RomType::GB:  return 524288;
        default:           return 8192;
    }
}

uint32_t SafetyValidator::readU32(const std::string& data, size_t offset) {
    if (offset + 4 > data.size()) return 0;

    return  (uint8_t)data[offset] |
           ((uint8_t)data[offset + 1] << 8) |
           ((uint8_t)data[offset + 2] << 16) |
           ((uint8_t)data[offset + 3] << 24);
}

ValidationResult SafetyValidator::validate(
    const std::string& data,
    size_t trimPoint,
    RomType romType,
    const TrimOptions& options)
{
    ValidationResult result;

    if (data.empty()) {
        result.isValid = false;
        result.message = tr("ROM data is empty");
        return result;
    }

    if (trimPoint >= data.size()) {
        result.isValid = false;
        result.message = tr("Trim point exceeds ROM size");
        return result;
    }

    bool ok = false;

    switch (romType) {
        case RomType::GBA:
            ok = validateGba(data, trimPoint);
            break;
        case RomType::NDS:
            ok = validateNds(data, trimPoint);
            break;
        case RomType::GB:
            ok = validateGb(data, trimPoint);
            break;
        default:
            result.isValid = false;
            result.message = tr("Unknown ROM type");
            return result;
    }

    if (!ok && !options.force) {
        result.isValid = false;
        result.message = tr("Safety validation failed");
        return result;
    }

    result.isValid = true;
    return result;
}