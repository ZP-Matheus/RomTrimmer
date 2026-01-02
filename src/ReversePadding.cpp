// ReversePadding.cpp
#include "ReversePadding.hpp"
#include <fstream>
#include <algorithm>
#include <cstring>

std::vector<uint8_t> ReversePadding::createRestorationPatch(
    const std::string& originalData,
    const std::string& trimmedData,
    uint8_t paddingByte) {
    
    std::vector<uint8_t> patch;
    
    // Simple implementation: just store the padding bytes
    if (originalData.size() <= trimmedData.size()) {
        return patch; // No padding to restore
    }
    
    size_t paddingSize = originalData.size() - trimmedData.size();
    
    // Verify it's actually padding
    for (size_t i = trimmedData.size(); i < originalData.size(); ++i) {
        if (static_cast<uint8_t>(originalData[i]) != paddingByte) {
            // Not pure padding, can't create simple patch
            return patch;
        }
    }
    
    // Create patch header
    uint32_t magic = PATCH_MAGIC;
    uint32_t dataSize = static_cast<uint32_t>(paddingSize);
    
    patch.resize(sizeof(magic) + 1 + sizeof(dataSize) + paddingSize);
    uint8_t* ptr = patch.data();
    
    // Write magic
    std::memcpy(ptr, &magic, sizeof(magic));
    ptr += sizeof(magic);
    
    // Write padding byte
    *ptr++ = paddingByte;
    
    // Write data size
    std::memcpy(ptr, &dataSize, sizeof(dataSize));
    ptr += sizeof(dataSize);
    
    // Write padding data (just the byte repeated)
    std::fill(ptr, ptr + paddingSize, paddingByte);
    
    return patch;
}

std::string ReversePadding::applyRestorationPatch(
    const std::string& trimmedData,
    const std::vector<uint8_t>& patch) {
    
    if (patch.size() < sizeof(uint32_t) + 1 + sizeof(uint32_t)) {
        return trimmedData; // Invalid patch
    }
    
    const uint8_t* ptr = patch.data();
    uint32_t magic;
    std::memcpy(&magic, ptr, sizeof(magic));
    ptr += sizeof(magic);
    
    if (magic != PATCH_MAGIC) {
        return trimmedData; // Invalid magic
    }
    
    uint8_t paddingByte = *ptr++;
    uint32_t paddingSize;
    std::memcpy(&paddingSize, ptr, sizeof(paddingSize));
    ptr += sizeof(paddingSize);
    
    // Create restored data
    std::string restored = trimmedData;
    restored.append(paddingSize, static_cast<char>(paddingByte));
    
    return restored;
}
