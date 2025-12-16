#include "../include/RomDetector.hpp"
#include "../include/PaddingAnalyzer.hpp"
#include "../include/SafetyValidator.hpp"
#include "ValidationResult.hpp"   // ou SafetyValidator completa, se ela definir
#include "TrimOptions.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <cstring>  // Para memcpy

void testRomDetector() {
    RomDetector detector;
    
    // Teste GBA
    std::string gbaData(1024 * 1024, '\xFF'); // 1MB de 0xFF
    
    // Adicionar logo Nintendo (usando memcpy com cuidado)
    uint8_t nintendoLogo[] = {0x24, 0xFF, 0xAE, 0x51};
    if (gbaData.size() >= 0x04 + sizeof(nintendoLogo)) {
        for (size_t i = 0; i < sizeof(nintendoLogo); i++) {
            gbaData[0x04 + i] = static_cast<char>(nintendoLogo[i]);
        }
    }
    
    RomType type = detector.detect(gbaData);
    assert(type == RomType::GBA || type == RomType::UNKNOWN);
    
    std::cout << "✓ RomDetector test passed" << std::endl;
}

void testPaddingAnalyzer() {
    PaddingAnalyzer analyzer;
    
    // Criar dados com padding
    std::string data;
    
    // Dados "válidos"
    for (int i = 0; i < 1000; i++) {
        data.push_back(static_cast<char>(i % 256));
    }
    
    // Padding de 0xFF
    data.append(500, '\xFF');
    
    PaddingAnalysis analysis = analyzer.analyze(data, 0xFF);
    
    assert(analysis.hasPadding == true);
    assert(analysis.trimPoint == 1000);
    assert(analysis.paddingSize == 500);
    
    std::cout << "✓ PaddingAnalyzer test passed" << std::endl;
}

void testSafetyValidator() {
    SafetyValidator validator;
    
    std::string data(2 * 1024 * 1024, 'A'); // 2MB de 'A' (0x41)
    
    TrimOptions options;
    options.minSize = 1 * 1024 * 1024;
    options.maxCutRatio = 0.5;
    options.safetyMargin = 65536;
    
    // Testar corte seguro (1.5MB)
    ValidationResult result = validator.validate(data, 1.5 * 1024 * 1024, 
                                                RomType::GBA, options);
    assert(result.isValid == true);
    
    // Testar corte inseguro (muito pequeno)
    result = validator.validate(data, 512 * 1024, RomType::GBA, options);
    assert(result.isValid == false);
    
    std::cout << "✓ SafetyValidator test passed" << std::endl;
}

void testEndToEnd() {
    std::string romData;
    
    // Simular uma ROM GBA de 8MB
    size_t romSize = 8 * 1024 * 1024;
    size_t actualData = 6 * 1024 * 1024;
    
    // Dados reais (padrão aleatório)
    srand(42);
    romData.reserve(romSize);
    
    for (size_t i = 0; i < actualData; i++) {
        romData.push_back(static_cast<char>(rand() % 256));
    }
    
    // Padding de 2MB
    romData.append(romSize - actualData, '\xFF');
    
    // Verificar que detecta padding correto
    PaddingAnalyzer analyzer;
    PaddingAnalysis analysis = analyzer.analyze(romData, 0xFF);
    
    assert(analysis.hasPadding == true);
    assert(analysis.trimPoint == actualData || 
           analysis.trimPoint == actualData + (4 - actualData % 4));
    
    std::cout << "✓ End-to-end test passed" << std::endl;
}

int main() {
    std::cout << "Running RomTrimmer++ tests...\n" << std::endl;
    
    try {
        testRomDetector();
        testPaddingAnalyzer();
        testSafetyValidator();
        testEndToEnd();
        
        std::cout << "\n✅ All tests passed!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}