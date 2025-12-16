#pragma once

#include <cxxopts.hpp>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <atomic>
#include <mutex>
#include <chrono>
#include <unordered_set>

#include "Logger.hpp"
#include "RomDetector.hpp"
#include "PaddingAnalyzer.hpp"
#include "SafetyValidator.hpp"
#include "ConfigManager.hpp"
#include "TrimOptions.hpp"

namespace fs = std::filesystem;

class RomTrimmer {
public:
    RomTrimmer();
    void run(int argc, char** argv);

private:
    struct FileStats {
        fs::path path;
        size_t originalSize = 0;
        size_t trimmedSize = 0;
        double savedRatio = 0.0;
        bool trimmed = false;
        std::string error;
        std::string romType;
        std::vector<std::string> warnings;
        std::chrono::steady_clock::time_point startTime;
        std::chrono::steady_clock::time_point endTime;
        std::chrono::milliseconds duration{0};
    };

    // Core
    TrimOptions options;
    std::unique_ptr<Logger> logger;
    std::unique_ptr<RomDetector> romDetector;
    std::unique_ptr<PaddingAnalyzer> paddingAnalyzer;
    std::unique_ptr<SafetyValidator> safetyValidator;
    std::unique_ptr<ConfigManager> configManager;
    
    std::chrono::steady_clock::time_point processingStartTime;

    // Argumentos
    bool parseArguments(int argc, char** argv);
    void printHelp(const cxxopts::Options& options) const;
    
    // Inicialização
    void initializeConfiguration();
    void applyDefaultConfiguration();
    void defineCommandLineOptions(cxxopts::Options& opts);
    void processCommandLineOptions(const cxxopts::ParseResult& result);
    bool validateOptions();

    // Pipeline
    bool collectFiles();
    void collectFilesFromDirectory(const fs::path& dir, std::vector<fs::path>& allFiles);
    bool isSupportedFileExtension(const fs::path& filePath);
    void removeDuplicatesAndSort(std::vector<fs::path>& files);
    
    void processFiles();
    bool processFile(const fs::path& filePath);
    uint8_t determinePaddingByte(const std::string& data, RomType romType);
    void handleValidationFailure(const ValidationResult& validation, FileStats& stats);
    bool executeFileAction(const fs::path& filePath, const std::string& data, 
                          size_t trimPoint, FileStats& stats);
    bool handleAnalysisMode(const std::string& data, size_t trimPoint, FileStats& stats);
    bool handleDryRunMode(const std::string& data, size_t trimPoint, FileStats& stats);
    bool handleActualTrim(const fs::path& filePath, const std::string& data, 
                         size_t trimPoint, FileStats& stats);
    void handleProcessingError(const fs::path& filePath, const std::string& error, 
                              FileStats& stats);

    // Operações de arquivo
    bool writeTrimmedFile(const fs::path& filePath, const std::string& data, size_t trimPoint);
    std::string readFile(const fs::path& filePath);
    fs::path determineOutputPath(const fs::path& inputPath);
    void createBackup(const fs::path& filePath) const;

    // Utilitários
    std::string formatBytes(size_t bytes) const;
    std::string romTypeToString(RomType type) const;
    void recordFileStats(FileStats& stats);

    // Resumo e estatísticas
    void printSummary() const;
    void printDetailedSummary() const;
    
    // Manipulação de erros
    void handleCriticalError(const std::string& error);
    void startProcessing();
    void cleanup();

    // Stats
    std::atomic<size_t> filesProcessed{0};
    std::atomic<size_t> filesTrimmed{0};
    std::atomic<size_t> filesFailed{0};
    std::atomic<size_t> totalSaved{0};

    std::mutex statsMutex;
    std::vector<FileStats> fileStats;
};