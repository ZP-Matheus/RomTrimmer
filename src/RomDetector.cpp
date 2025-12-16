#include "RomDetector.hpp"

#include <string>
#include <cstddef>
#include <cstdint>   // 👈 O CARA QUE TAVA FALTANDO
#include <cstring>

RomType RomDetector::detect(const std::string& data) {
    if (data.size() < 192) { // Tamanho mínimo para header
        return RomType::UNKNOWN;
    }
    
    // Verificar GBA
    if (isGbaRom(data)) {
        return RomType::GBA;
    }
    
    // Verificar NDS
    if (isNdsRom(data)) {
        return RomType::NDS;
    }
    
    // Verificar GB/GBC
    if (isGbRom(data)) {
        return RomType::GB;
    }
    
    return RomType::UNKNOWN;
}

bool RomDetector::isGbaRom(const std::string& data) {
    // Logo Nintendo em 0x04 - 0x9F
    const uint8_t nintendoLogo[] = {
        0x24, 0xFF, 0xAE, 0x51, 0x69, 0x9A, 0xA2, 0x21, 0x3D, 0x84, 0x82, 0x0A,
        0x84, 0xE4, 0x09, 0xAD, 0x11, 0x24, 0x8B, 0x98, 0xC0, 0x81, 0x7F, 0x21,
        0xA3, 0x52, 0xBE, 0x19, 0x93, 0x09, 0xCE, 0x20, 0x10, 0x46, 0x4A, 0x4A,
        0xF8, 0x27, 0x31, 0xEC, 0x58, 0xC7, 0xE8, 0x33, 0x82, 0xE3, 0xCE, 0xBF,
        0x85, 0xF4, 0xDF, 0x94, 0xCE, 0x4B, 0x09, 0xC1, 0x94, 0x56, 0x8A, 0xC0,
        0x13, 0x72, 0xA7, 0xFC, 0x9F, 0x84, 0x4D, 0x73, 0xA3, 0xCA, 0x9A, 0x61,
        0x58, 0x97, 0xA3, 0x27, 0xFC, 0x03, 0x98, 0x76, 0x23, 0x1D, 0xC7, 0x61,
        0x03, 0x04, 0xAE, 0x56, 0xBF, 0x38, 0x84, 0x00, 0x40, 0xA7, 0x0E, 0xFD,
        0xFF, 0x52, 0xFE, 0x03, 0x6F, 0x95, 0x30, 0xF1, 0x97, 0xFB, 0xC0, 0x85,
        0x60, 0xD6, 0x80, 0x25, 0xA9, 0x63, 0xBE, 0x03, 0x01, 0x4E, 0x38, 0xE2,
        0xF9, 0xA2, 0x34, 0xFF, 0xBB, 0x3E, 0x03, 0x44, 0x78, 0x00, 0x90, 0xCB,
        0x88, 0x11, 0x3A, 0x94, 0x65, 0xC0, 0x7C, 0x63, 0x87, 0xF0, 0x3C, 0xAF,
        0xD6, 0x25, 0xE4, 0x8B, 0x38, 0x0A, 0xAC, 0x72, 0x21, 0xD4, 0xF8, 0x07
    };
    
    if (memcmp(&data[0x04], nintendoLogo, sizeof(nintendoLogo)) == 0) {
        return true;
    }
    
    // Verificar tamanho típico de ROMs GBA (potências de 2)
    size_t size = data.size();
    if (size >= 1024 * 1024 && size <= 32 * 1024 * 1024) {
        // Verificar se termina com 0xFF ou 0x00 (comum em ROMs GBA)
        size_t lastNonPadding = findLastNonPadding(data, 0xFF);
        if (lastNonPadding < size) {
            // Tamanho após trim seria potência de 2?
            size_t trimmedSize = lastNonPadding + 1;
            if (isPowerOfTwo(trimmedSize) || 
                (trimmedSize % (1024 * 1024) == 0)) {
                return true;
            }
        }
    }
    
    return false;
}

bool RomDetector::isNdsRom(const std::string& data) {
    if (data.size() < 512) {
        return false;
    }
    
    // Verificar assinatura "Nintendo DS" no header
    const char* ndsSignature = "Nintendo DS";
    if (memcmp(&data[0x0C], ndsSignature, 12) == 0) {
        return true;
    }
    
    // Verificar offsets ARM9/ARM7
    uint32_t arm9Offset, arm7Offset;
    
    // Extrair os 4 bytes do offset (little-endian)
    arm9Offset = static_cast<uint8_t>(data[0x20]) |
                 (static_cast<uint8_t>(data[0x21]) << 8) |
                 (static_cast<uint8_t>(data[0x22]) << 16) |
                 (static_cast<uint8_t>(data[0x23]) << 24);
    
    arm7Offset = static_cast<uint8_t>(data[0x30]) |
                 (static_cast<uint8_t>(data[0x31]) << 8) |
                 (static_cast<uint8_t>(data[0x32]) << 16) |
                 (static_cast<uint8_t>(data[0x33]) << 24);
    
    if (arm9Offset < data.size() && arm7Offset < data.size()) {
        // Offsets devem ser múltiplos de 4
        if (arm9Offset % 4 == 0 && arm7Offset % 4 == 0) {
            return true;
        }
    }
    
    return false;
}

bool RomDetector::isGbRom(const std::string& data) {
    if (data.size() < 0x150) {
        return false;
    }
    
    // Verificar logo Nintendo (GB/GBC)
    const uint8_t gbLogo[] = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83,
        0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
        0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63,
        0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
    };
    
    if (memcmp(&data[0x104], gbLogo, sizeof(gbLogo)) == 0) {
        return true;
    }
    
    return false;
}

size_t RomDetector::findLastNonPadding(const std::string& data, uint8_t padding) {
    for (size_t i = data.size(); i > 0; --i) {
        if (static_cast<uint8_t>(data[i - 1]) != padding) {
            return i - 1;
        }
    }
    return 0;
}

bool RomDetector::isPowerOfTwo(size_t n) {
    return (n != 0) && ((n & (n - 1)) == 0);
}