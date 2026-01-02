// DatIntegration.hpp - Updated version
#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct RomEntry {
    std::string name;
    std::string size;
    std::string crc32;
    std::string md5;
    std::string sha1;
    std::string status;  // "ok", "missing", "modified", "only_in_first", "only_in_second"
};

struct RomSetStats {
    size_t totalGames = 0;
    size_t totalSize = 0;
    size_t averageSize = 0;
    size_t largestRom = 0;
    std::string largestRomName;
    size_t smallestRom = 0;
    std::string smallestRomName;
    size_t verified = 0;
    size_t missing = 0;
    size_t modified = 0;
};

class DatIntegrator {
public:
    // ==================== CORE DAT OPERATIONS ====================
    
    // Parse DAT file (Logiqx XML or ClrMamePro format)
    static std::vector<RomEntry> parseDatFile(const std::string& datPath);
    
    // Verify ROM against DAT entry
    static bool verifyRom(const std::string& romPath, const RomEntry& entry);
    
    // Calculate all checksums for a file
    static std::unordered_map<std::string, std::string> calculateChecksums(
        const std::string& filePath);
    
    // Generate trimmed DAT file with updated checksums
    static bool generateTrimmedDat(
        const std::vector<RomEntry>& originalEntries,
        const std::string& outputDatPath,
        const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& trimmedChecksums);
    
    // ==================== BATCH OPERATIONS ====================
    
    // Verify all files in directory against DAT
    static std::unordered_map<std::string, RomEntry> verifyDirectoryAgainstDat(
        const std::string& directoryPath,
        const std::vector<RomEntry>& datEntries,
        bool recursive = false);
    
    // Generate patch DAT (IPS format)
    static bool generatePatchDat(
        const std::vector<RomEntry>& originalEntries,
        const std::string& patchDirectory,
        const std::string& outputDatPath);
    
    // Compare two DAT files
    static std::vector<RomEntry> diffDatFiles(
        const std::vector<RomEntry>& dat1,
        const std::vector<RomEntry>& dat2);
    
    // Rename files to match DAT names
    static bool renameFilesToDatNames(
        const std::string& directoryPath,
        const std::vector<RomEntry>& datEntries,
        bool dryRun = false);
    
    // ==================== EXTENDED FEATURES ====================
    
    // Export to CSV format
    static bool exportToCsv(const std::vector<RomEntry>& entries,
                           const std::string& csvPath);
    
    // Import from CSV format
    static std::vector<RomEntry> importFromCsv(const std::string& csvPath);
    
    // Calculate ROM set statistics
    static RomSetStats calculateRomSetStats(
        const std::vector<RomEntry>& entries);
    
    // Validate DAT file structure
    static bool validateDatFile(const std::string& datPath,
                               std::vector<std::string>& errors);
    
    // ==================== INTEGRATION WITH ROMTRIMMER ====================
    
    // Complete workflow: parse DAT, verify, trim, generate new DAT
    static std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
    processDirectoryWithDat(
        const std::string& directoryPath,
        const std::string& datPath,
        bool trimFiles = true,
        const std::string& outputDir = "");
    
private:
    // Checksum calculation algorithms
    static std::string calculateCRC32(const std::string& data);
    static std::string calculateMD5(const std::string& data);
    static std::string calculateSHA1(const std::string& data);
    
    // Helper functions
    static std::string escapeXml(const std::string& input);
    static std::string getCurrentTimestamp();
    static std::string toLower(const std::string& str);
    static size_t countSubstring(const std::string& str,
                                const std::string& substr);
};
