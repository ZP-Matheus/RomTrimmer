#pragma once
#include <vector>
#include <filesystem>
#include <cstdint>
#include <string>
#include <sstream>      // ADICIONE ESTE
#include <iomanip>      // ADICIONE ESTE
#include <unordered_map>

namespace fs = std::filesystem;

struct TrimOptions {
    // ==================== MODOS DE OPERAÇÃO ====================
    bool recursive = false;
    bool dryRun = false;
    bool backup = true;
    bool verbose = false;
    bool analyzeOnly = false;
    bool force = false;
    bool helpRequested = false;
    bool versionRequested = false;
    
    // ==================== CONFIGURAÇÕES DE PADDING ====================
    uint8_t paddingByte = 0xFF; // 0xFF, 0x00, or 0 for auto
    
    // ==================== CONFIGURAÇÕES DE SEGURANÇA ====================
    size_t minSize = 1024;
    size_t safetyMargin = 65536;
    double maxCutRatio = 0.6;
    
    // ==================== CONFIGURAÇÕES DE SAÍDA ====================
    fs::path outputDir;
    std::vector<fs::path> inputPaths;
    
    // ==================== MÉTODOS UTILITÁRIOS ====================
    std::string toString() const {
        std::stringstream ss;
        ss << "TrimOptions {\n"
           << "  recursive: " << recursive << "\n"
           << "  dryRun: " << dryRun << "\n"
           << "  backup: " << backup << "\n"
           << "  verbose: " << verbose << "\n"
           << "  analyzeOnly: " << analyzeOnly << "\n"
           << "  force: " << force << "\n"
           << "  paddingByte: 0x" << std::hex << static_cast<int>(paddingByte) << std::dec << "\n"
           << "  minSize: " << minSize << " bytes\n"
           << "  safetyMargin: " << safetyMargin << " bytes\n"
           << "  maxCutRatio: " << (maxCutRatio * 100) << "%\n"
           << "  outputDir: " << (outputDir.empty() ? "(none)" : outputDir.string()) << "\n"
           << "  inputPaths: " << inputPaths.size() << " paths\n"
           << "}";
        return ss.str();
    }
    
    std::string toCommandLine() const {
        std::stringstream ss;
        
        if (recursive) ss << " -r";
        if (dryRun) ss << " --dry-run";
        if (!backup) ss << " --no-backup";
        if (verbose) ss << " -v";
        if (analyzeOnly) ss << " --analyze";
        if (force) ss << " --force";
        
        if (paddingByte != 0xFF) {
            if (paddingByte == 0x00) ss << " --padding-byte 0x00";
            else ss << " --padding-byte auto";
        }
        
        if (minSize != 1024) ss << " --min-size " << minSize;
        if (safetyMargin != 65536) ss << " --safety-margin " << safetyMargin;
        if (maxCutRatio != 0.6) ss << " --max-cut-ratio " << maxCutRatio;
        
        if (!outputDir.empty()) ss << " -o \"" << outputDir.string() << "\"";
        
        return ss.str();
    }
};