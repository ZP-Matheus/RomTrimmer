#pragma once

#include <vector>
#include <filesystem>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

// =====================
// TrimOptions
// =====================
struct TrimOptions {
    // ==================== MODOS DE OPERAÇÃO ====================
    bool recursive        = false;
    bool dryRun           = false;
    bool backup           = true;
    bool verbose          = false;
    bool analyzeOnly      = false;
    bool force            = false;
    bool helpRequested    = false;
    bool versionRequested = false;

    // ==================== CONFIGURAÇÕES DE PADDING ====================
    // 0xFF = padrão, 0x00 = zero-fill, 0xFE = auto
    uint8_t paddingByte = 0xFF;

    // ==================== CONFIGURAÇÕES DE SEGURANÇA ====================
    size_t minSize       = 1024;
    size_t safetyMargin  = 64 * 1024;
    double maxCutRatio   = 0.6;

    // ==================== CONFIGURAÇÕES DE SAÍDA ====================
    fs::path outputDir;
    std::vector<fs::path> inputPaths;

    // ==================== UTILITÁRIOS ====================
    std::string toString() const {
        std::ostringstream ss;

        ss << "TrimOptions {\n"
           << "  recursive: "        << recursive        << "\n"
           << "  dryRun: "           << dryRun           << "\n"
           << "  backup: "           << backup           << "\n"
           << "  verbose: "          << verbose          << "\n"
           << "  analyzeOnly: "      << analyzeOnly      << "\n"
           << "  force: "            << force            << "\n"
           << "  paddingByte: 0x"
           << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<int>(paddingByte)
           << std::dec << "\n"
           << "  minSize: "          << minSize          << " bytes\n"
           << "  safetyMargin: "     << safetyMargin     << " bytes\n"
           << "  maxCutRatio: "      << (maxCutRatio * 100.0) << "%\n"
           << "  outputDir: "        << (outputDir.empty() ? "(none)" : outputDir.string()) << "\n"
           << "  inputPaths: "       << inputPaths.size() << " paths\n"
           << "}";

        return ss.str();
    }

    std::string toCommandLine() const {
        std::ostringstream ss;

        if (recursive)   ss << " -r";
        if (dryRun)      ss << " --dry-run";
        if (!backup)     ss << " --no-backup";
        if (verbose)     ss << " -v";
        if (analyzeOnly) ss << " --analyze";
        if (force)       ss << " --force";

        if (paddingByte != 0xFF) {
            ss << " --padding-byte ";
            if (paddingByte == 0x00)
                ss << "0x00";
            else
                ss << "auto";
        }

        if (minSize != 1024)
            ss << " --min-size " << minSize;

        if (safetyMargin != 64 * 1024)
            ss << " --safety-margin " << safetyMargin;

        if (maxCutRatio != 0.6)
            ss << " --max-cut-ratio " << maxCutRatio;

        if (!outputDir.empty())
            ss << " -o \"" << outputDir.string() << "\"";

        return ss.str();
    }
};