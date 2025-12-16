#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <array>

class ChecksumVerifier {
public:
    enum class ChecksumType {
        CRC32,
        MD5,
        SHA1,
        SHA256
    };
    
    struct ChecksumResult {
        bool valid = false;
        std::string expected;
        std::string actual;
        ChecksumType type;
    };
    
    ChecksumVerifier() = default;
    ~ChecksumVerifier() = default;
    
    // Calcular checksum
    std::string calculate(const std::string& data, ChecksumType type);
    
    // Verificar contra valor esperado
    ChecksumResult verify(const std::string& data, 
                         const std::string& expected, 
                         ChecksumType type);
    
    // Verificar arquivo contra database conhecido
    bool verifyAgainstDatabase(const std::string& filename, 
                              const std::string& data);
    
private:
    std::array<uint8_t, 16> calculateMD5(const std::string& data);
    std::array<uint8_t, 20> calculateSHA1(const std::string& data);
    std::array<uint8_t, 32> calculateSHA256(const std::string& data);
    uint32_t calculateCRC32(const std::string& data);
    
    std::string bytesToHex(const uint8_t* bytes, size_t length);
};