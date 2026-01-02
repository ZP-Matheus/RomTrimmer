// romtrimmer_c.cpp - C++ implementation of C interface
#include "romtrimmer_c.h"
#include "RomTrimmer.hpp"
#include "PaddingAnalyzer.hpp"
#include "RomDetector.hpp"
#include "SafetyValidator.hpp"

#include <memory>
#include <vector>

struct rt_context {
    std::unique_ptr<RomTrimmer> trimmer;
    std::unique_ptr<RomDetector> detector;
    std::unique_ptr<PaddingAnalyzer> analyzer;
    std::unique_ptr<SafetyValidator> validator;
};

// Simple C++ to C wrapper implementation
extern "C" {
    
rt_error_t rt_analyze_file(const char* filename, rt_analysis_result_t* result) {
    if (!filename || !result) return RT_ERROR_INVALID_PARAM;
    
    try {
        // Read file
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file) return RT_ERROR_FILE_NOT_FOUND;
        
        size_t size = file.tellg();
        file.seekg(0);
        std::vector<uint8_t> data(size);
        file.read(reinterpret_cast<char*>(data.data()), size);
        
        // Convert to string for existing API
        std::string data_str(data.begin(), data.end());
        
        // Detect ROM type
        RomDetector detector;
        RomType romType = detector.detect(data_str);
        
        // Map to C enum
        switch (romType) {
            case RomType::GBA: result->rom_type = RT_ROM_GBA; break;
            case RomType::NDS: result->rom_type = RT_ROM_NDS; break;
            case RomType::GB: result->rom_type = RT_ROM_GB; break;
            case RomType::GBC: result->rom_type = RT_ROM_GBC; break;
            default: result->rom_type = RT_ROM_UNKNOWN; break;
        }
        
        // Analyze padding
        PaddingAnalyzer analyzer;
        auto padding_byte = analyzer.autoDetectPadding(data_str, romType);
        auto analysis = analyzer.analyze(data_str, padding_byte);
        
        // Fill result
        result->original_size = data_str.size();
        result->has_padding = analysis.hasPadding;
        result->trimmed_size = analysis.trimPoint;
        result->padding_bytes = analysis.paddingSize;
        result->saved_percentage = 100.0 * (1.0 - (double)result->trimmed_size / result->original_size);
        
        return RT_SUCCESS;
        
    } catch (...) {
        return RT_ERROR_READ_FAILED;
    }
}
    
// Other C interface implementations...
}
