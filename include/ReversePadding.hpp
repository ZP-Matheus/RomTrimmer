// ReversePadding.hpp
#pragma once
#include <string>
#include <vector>
#include <cstdint>

class ReversePadding {
public:
    // Patch format
    struct PatchEntry {
        uint32_t offset;
        uint32_t length;
        uint8_t value;
        bool is_rle;  // Run-length encoded
    };
    
    // Create patch to restore padding
    static std::vector<uint8_t> createRestorationPatch(
        const std::string& originalData,
        const std::string& trimmedData,
        uint8_t paddingByte);
    
    // Apply patch to restore padding
    static std::string applyRestorationPatch(
        const std::string& trimmedData,
        const std::vector<uint8_t>& patch);
    
    // Simple restoration (just add padding bytes)
    static std::string restorePaddingSimple(
        const std::string& trimmedData,
        size_t originalSize,
        uint8_t paddingByte);
    
    // Save patch to file (custom format)
    static bool savePatch(const std::vector<uint8_t>& patch, 
                         const std::string& filename);
    
    // Load patch from file
    static std::vector<uint8_t> loadPatch(const std::string& filename);
    
private:
    // Simple patch format:
    // [4 bytes: magic "RTPT"]
    // [1 byte: padding value]
    // [4 bytes: patch data size]
    // [patch data...]
    static const uint32_t PATCH_MAGIC = 0x54505452; // "RTPT" in little-endian
};
