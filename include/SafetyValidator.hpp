#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "RomDetector.hpp"

// Forward declaration
struct TrimOptions;
struct ValidationResult;

class SafetyValidator {
public:
    SafetyValidator() = default;
    ~SafetyValidator() = default;
    
    // Método principal de validação
    ValidationResult validate(const std::string& data,
                             size_t trimPoint,
                             RomType romType,
                             const TrimOptions& options);
    
    // Validações específicas por tipo de ROM (públicas para testes)
    bool validateGba(const std::string& data, size_t trimPoint);
    bool validateNds(const std::string& data, size_t trimPoint);
    bool validateGb(const std::string& data, size_t trimPoint);
    
    // Validações internas usadas pelo método principal
    bool validateMinimumSize(size_t trimPoint, RomType romType, 
                             const TrimOptions& options, ValidationResult& result);
    bool validateCutRatio(const std::string& data, size_t trimPoint,
                          const TrimOptions& options, ValidationResult& result);
    bool validateSafetyMargin(size_t trimPoint, const TrimOptions& options,
                              ValidationResult& result);
    bool validateRomSpecific(const std::string& data, size_t trimPoint,
                             RomType romType, ValidationResult& result);
    bool validateKnownStructures(const std::string& data, size_t trimPoint,
                                 RomType romType, ValidationResult& result);
    bool validateHeaderIntegrity(const std::string& data, size_t trimPoint,
                                 RomType romType, ValidationResult& result);
    
    // Métodos auxiliares
    size_t getMinSizeForRomType(RomType type);
    size_t getRecommendedSizeForRomType(RomType type);
    
    // Análise de risco
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
    // Funções privadas de validação
    bool validateGbaInternalRomSize(const std::string& data, size_t trimPoint);
    bool validateNdsSectionOffsets(const std::string& data, size_t trimPoint);
    bool validateGbRomSize(size_t size);
    
    // Funções auxiliares internas
    bool validateKnownStructuresInternal(const std::string& data,
                                        size_t trimPoint,
                                        RomType romType);
    std::string analyzeStructureConflict(const std::string& data,
                                        size_t trimPoint,
                                        RomType romType);
    bool validateHeaderIntegrityInternal(const std::string& data,
                                        size_t trimPoint,
                                        RomType romType);
    
    // Validações de header
    bool validateGbaHeader(const std::string& data, size_t trimPoint);
    bool validateNdsHeader(const std::string& data, size_t trimPoint);
    bool validateGbHeader(const std::string& data, size_t trimPoint);
    
    // Cálculos de checksum
    uint8_t calculateGbaChecksum(const std::string& data);
    uint8_t calculateGbChecksum(const std::string& data);
    
    // Leitura de dados
    uint32_t readU32(const std::string& data, size_t offset);
    
    // Verificações de dados
    bool containsKnownDataPatterns(const std::string& data,
                                  size_t start,
                                  size_t end);
    
    // Constantes de validação
    static constexpr size_t MIN_GBA_SIZE = 1024 * 1024;       // 1MB
    static constexpr size_t MIN_NDS_SIZE = 8 * 1024 * 1024;   // 8MB
    static constexpr size_t MIN_GB_SIZE = 32768;              // 32KB
    
    static constexpr double MAX_CUT_RATIO_DEFAULT = 0.6;      // 60%
    static constexpr size_t SAFETY_MARGIN_DEFAULT = 65536;    // 64KB
};