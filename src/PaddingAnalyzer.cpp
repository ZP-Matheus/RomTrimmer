#include "PaddingAnalyzer.hpp"
#include <algorithm>
#include <cmath>

PaddingAnalysis PaddingAnalyzer::analyze(const std::string& data, 
                                        uint8_t paddingByte) {
    PaddingAnalysis result;
    result.hasPadding = false;
    result.trimPoint = data.size();
    result.paddingByte = paddingByte;
    result.confidence = 0.0;
    
    if (data.empty()) {
        return result;
    }
    
    // Encontrar o último byte não-padding
    size_t lastNonPadding = data.size() - 1;
    while (lastNonPadding > 0 && static_cast<uint8_t>(data[lastNonPadding]) == paddingByte) {
        --lastNonPadding;
    }
    
    // Se o último byte também é padding, todo o arquivo é padding?
    if (static_cast<uint8_t>(data[lastNonPadding]) == paddingByte) {
        result.hasPadding = false; // Arquivo inteiro é padding? Impossível
        return result;
    }
    
    // Calcular quantos bytes de padding no final
    size_t paddingBytes = data.size() - lastNonPadding - 1;
    
    if (paddingBytes == 0) {
        return result;
    }
    
    // Verificar se é padding "puro" (sequência contínua)
    bool isPurePadding = true;
    for (size_t i = lastNonPadding + 1; i < data.size(); ++i) {
        if (static_cast<uint8_t>(data[i]) != paddingByte) {
            isPurePadding = false;
            break;
        }
    }
    
    if (!isPurePadding) {
        // Padding interrompido - pode ser dados válidos
        return result;
    }
    
    // Verificar padrões alternados (sinal de possíveis dados)
    if (hasAlternatingPattern(data, paddingByte)) {
        result.confidence = 0.3;
        return result;
    }
    
    // Calcular confiança baseada no tamanho do padding
    double paddingRatio = static_cast<double>(paddingBytes) / data.size();
    
    // Padding muito pequeno (menos de 1KB) pode ser intencional
    if (paddingBytes < 1024) {
        result.confidence = 0.5;
    }
    // Padding muito grande (mais de 50%) é suspeito
    else if (paddingRatio > 0.5) {
        result.confidence = 0.7;
    }
    else {
        result.confidence = 0.9;
    }
    
    // Ajustar confiança baseado no tipo de ROM
    result.confidence = adjustConfidenceForRomType(result.confidence, 
                                                  paddingBytes, data.size());
    
    result.hasPadding = true;
    result.trimPoint = lastNonPadding + 1;
    result.paddingSize = paddingBytes;
    
    // Arredondar para múltiplo de 4 (alinhamento comum)
    if (result.trimPoint % 4 != 0) {
        result.trimPoint += (4 - (result.trimPoint % 4));
        if (result.trimPoint > data.size()) {
            result.trimPoint = data.size();
        }
    }
    
    return result;
}

uint8_t PaddingAnalyzer::autoDetectPadding(const std::string& data, 
                                          RomType romType) {
    // Contar ocorrências de 0xFF e 0x00 nos últimos 1KB
    size_t sampleSize = std::min<size_t>(data.size(), 1024);
    size_t ffCount = 0;
    size_t zeroCount = 0;
    
    for (size_t i = data.size() - sampleSize; i < data.size(); ++i) {
        if (static_cast<uint8_t>(data[i]) == 0xFF) ffCount++;
        if (static_cast<uint8_t>(data[i]) == 0x00) zeroCount++;
    }
    
    // ROMs GBA geralmente usam 0xFF, algumas DS usam 0x00
    if (romType == RomType::GBA) {
        return 0xFF;
    }
    else if (romType == RomType::NDS) {
        // DS pode usar ambos, mas 0xFF é mais comum
        return (ffCount > zeroCount) ? 0xFF : 0x00;
    }
    
    // Padrão geral: usar o que aparecer mais
    return (ffCount > zeroCount) ? 0xFF : 0x00;
}

bool PaddingAnalyzer::hasAlternatingPattern(const std::string& data, 
                                           uint8_t paddingByte) {
    // Verificar padrões como FF 00 FF 00 ou 00 FF 00 FF
    size_t checkSize = std::min<size_t>(data.size(), 256);
    
    if (checkSize < 4) return false;
    
    uint8_t alternate1 = paddingByte;
    uint8_t alternate2 = (paddingByte == 0xFF) ? 0x00 : 0xFF;
    
    bool pattern1 = true; // alternate1, alternate2, alternate1, alternate2...
    bool pattern2 = true; // alternate2, alternate1, alternate2, alternate1...
    
    for (size_t i = data.size() - checkSize; i < data.size(); ++i) {
        size_t pos = i - (data.size() - checkSize);
        uint8_t expected1 = (pos % 2 == 0) ? alternate1 : alternate2;
        uint8_t expected2 = (pos % 2 == 0) ? alternate2 : alternate1;
        
        if (static_cast<uint8_t>(data[i]) != expected1) pattern1 = false;
        if (static_cast<uint8_t>(data[i]) != expected2) pattern2 = false;
        
        if (!pattern1 && !pattern2) break;
    }
    
    return pattern1 || pattern2;
}

double PaddingAnalyzer::adjustConfidenceForRomType(double baseConfidence, 
                                                  size_t paddingSize, 
                                                  size_t totalSize) {
    // GBA: padding geralmente é múltiplo de 1MB
    // DS: padding geralmente é múltiplo de 4KB ou 8KB
    
    double sizeRatio = static_cast<double>(paddingSize) / totalSize;
    
    // Se o tamanho após trim seria "estranho" (não múltiplo de algo comum),
    // reduzir confiança
    size_t trimmedSize = totalSize - paddingSize;
    
    if (trimmedSize % (1024 * 1024) == 0) {
        // Múltiplo de 1MB - comum em GBA
        return std::min(baseConfidence + 0.1, 1.0);
    }
    else if (trimmedSize % 8192 == 0) {
        // Múltiplo de 8KB - comum em várias ROMs
        return std::min(baseConfidence + 0.05, 1.0);
    }
    else if (trimmedSize % 4096 != 0) {
        // Não é múltiplo de 4KB - incomum
        return std::max(baseConfidence - 0.2, 0.0);
    }
    
    return baseConfidence;
}