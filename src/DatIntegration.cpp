// DatIntegration.cpp
#include "DatIntegration.hpp"
#include "ChecksumVerifier.hpp"
#include <zlib.h>
#include <fstream>
#include <sstream>
#include <regex>
#include <iomanip>
#include <filesystem>
#include <algorithm>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <zlib.h>

namespace fs = std::filesystem;

// ==================== DAT PARSING ====================

std::vector<RomEntry> DatIntegrator::parseDatFile(const std::string& datPath) {
    std::vector<RomEntry> entries;
    std::ifstream file(datPath);
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open DAT file: " + datPath);
    }
    
    std::string line;
    RomEntry currentEntry;
    bool inGame = false;
    bool inRom = false;
    
    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (line.empty()) continue;
        
        // Convert to lowercase for case-insensitive parsing
        std::string lineLower = line;
        std::transform(lineLower.begin(), lineLower.end(), lineLower.begin(), ::tolower);
        
        // Check for game block start
        if (lineLower.find("<game") != std::string::npos) {
            inGame = true;
            currentEntry = RomEntry();
            
            // Extract game name from attribute
            std::regex nameRegex("name=\"([^\"]+)\"");
            std::smatch nameMatch;
            if (std::regex_search(line, nameMatch, nameRegex)) {
                currentEntry.name = nameMatch[1].str();
            }
            continue;
        }
        
        // Check for game block end
        if (lineLower.find("</game>") != std::string::npos) {
            if (!currentEntry.name.empty()) {
                entries.push_back(currentEntry);
            }
            inGame = false;
            continue;
        }
        
        // Check for rom block start
        if (inGame && lineLower.find("<rom") != std::string::npos) {
            inRom = true;
            
            // Extract ROM attributes
            std::regex sizeRegex("size=\"([^\"]+)\"");
            std::regex crcRegex("crc=\"([^\"]+)\"");
            std::regex md5Regex("md5=\"([^\"]+)\"");
            std::regex sha1Regex("sha1=\"([^\"]+)\"");
            std::regex nameRegex("name=\"([^\"]+)\"");
            
            std::smatch match;
            
            if (std::regex_search(line, match, sizeRegex)) {
                currentEntry.size = match[1].str();
            }
            if (std::regex_search(line, match, crcRegex)) {
                currentEntry.crc32 = match[1].str();
            }
            if (std::regex_search(line, match, md5Regex)) {
                currentEntry.md5 = match[1].str();
            }
            if (std::regex_search(line, match, sha1Regex)) {
                currentEntry.sha1 = match[1].str();
            }
            if (std::regex_search(line, match, nameRegex)) {
                // Use ROM name if game name not found
                if (currentEntry.name.empty()) {
                    currentEntry.name = match[1].str();
                }
            }
            continue;
        }
        
        // Check for rom block end
        if (inRom && lineLower.find("/>") != std::string::npos) {
            inRom = false;
            continue;
        }
        
        // Alternative format: ClrMamePro style
        if (lineLower.find("rom (") == 0) {
            // Format: rom ( name size crc md5 sha1 )
            std::istringstream iss(line);
            std::string token;
            std::vector<std::string> tokens;
            
            while (iss >> token) {
                tokens.push_back(token);
            }
            
            if (tokens.size() >= 6) {
                currentEntry.name = tokens[1];
                currentEntry.size = tokens[2];
                currentEntry.crc32 = tokens[3];
                currentEntry.md5 = tokens[4];
                currentEntry.sha1 = tokens[5];
                entries.push_back(currentEntry);
            }
        }
    }
    
    file.close();
    return entries;
}

// ==================== ROM VERIFICATION ====================

bool DatIntegrator::verifyRom(const std::string& romPath, const RomEntry& entry) {
    try {
        // Read ROM file
        std::ifstream file(romPath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return false;
        }
        
        size_t fileSize = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);
        
        std::vector<uint8_t> buffer(fileSize);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();
        
        std::string data(buffer.begin(), buffer.end());
        
        // Verify size
        if (!entry.size.empty()) {
            size_t expectedSize = std::stoull(entry.size);
            if (fileSize != expectedSize) {
                return false;
            }
        }
        
        // Verify CRC32
        if (!entry.crc32.empty()) {
            std::string calculatedCrc = calculateCRC32(data);
            std::string expectedCrc = entry.crc32;
            
            // Convert to lowercase for case-insensitive comparison
            std::transform(calculatedCrc.begin(), calculatedCrc.end(), calculatedCrc.begin(), ::tolower);
            std::transform(expectedCrc.begin(), expectedCrc.end(), expectedCrc.begin(), ::tolower);
            
            if (calculatedCrc != expectedCrc) {
                return false;
            }
        }
        
        // Verify MD5
        if (!entry.md5.empty()) {
            std::string calculatedMd5 = calculateMD5(data);
            std::string expectedMd5 = entry.md5;
            
            std::transform(calculatedMd5.begin(), calculatedMd5.end(), calculatedMd5.begin(), ::tolower);
            std::transform(expectedMd5.begin(), expectedMd5.end(), expectedMd5.begin(), ::tolower);
            
            if (calculatedMd5 != expectedMd5) {
                return false;
            }
        }
        
        // Verify SHA1
        if (!entry.sha1.empty()) {
            std::string calculatedSha1 = calculateSHA1(data);
            std::string expectedSha1 = entry.sha1;
            
            std::transform(calculatedSha1.begin(), calculatedSha1.end(), calculatedSha1.begin(), ::tolower);
            std::transform(expectedSha1.begin(), expectedSha1.end(), expectedSha1.begin(), ::tolower);
            
            if (calculatedSha1 != expectedSha1) {
                return false;
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

// ==================== CHECKSUM CALCULATION ====================

std::unordered_map<std::string, std::string> DatIntegrator::calculateChecksums(
    const std::string& filePath) {
    
    std::unordered_map<std::string, std::string> checksums;
    
    try {
        // Read file
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return checksums;
        }
        
        size_t fileSize = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);
        
        std::vector<uint8_t> buffer(fileSize);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();
        
        std::string data(buffer.begin(), buffer.end());
        
        // Calculate checksums
        checksums["size"] = std::to_string(fileSize);
        checksums["crc32"] = calculateCRC32(data);
        checksums["md5"] = calculateMD5(data);
        checksums["sha1"] = calculateSHA1(data);
        
    } catch (const std::exception& e) {
        // Return empty map on error
    }
    
    return checksums;
}

// ==================== CHECKSUM ALGORITHMS ====================

std::string DatIntegrator::calculateCRC32(const std::string& data) {
    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, reinterpret_cast<const Bytef*>(data.data()), data.size());
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(8) << crc;
    return ss.str();
}

std::string DatIntegrator::calculateMD5(const std::string& data) {
    MD5_CTX context;
    MD5_Init(&context);
    MD5_Update(&context, data.data(), data.size());
    
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_Final(digest, &context);
    
    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setfill('0') << std::setw(2) 
           << static_cast<unsigned int>(digest[i]);
    }
    
    return ss.str();
}

std::string DatIntegrator::calculateSHA1(const std::string& data) {
    SHA_CTX context;
    SHA1_Init(&context);
    SHA1_Update(&context, data.data(), data.size());
    
    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1_Final(digest, &context);
    
    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setfill('0') << std::setw(2) 
           << static_cast<unsigned int>(digest[i]);
    }
    
    return ss.str();
}

// ==================== DAT GENERATION ====================

bool DatIntegrator::generateTrimmedDat(
    const std::vector<RomEntry>& originalEntries,
    const std::string& outputDatPath,
    const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& trimmedChecksums) {
    
    try {
        std::ofstream outFile(outputDatPath);
        if (!outFile.is_open()) {
            return false;
        }
        
        // Write DAT header
        outFile << "<?xml version=\"1.0\"?>\n";
        outFile << "<!DOCTYPE datafile PUBLIC \"-//Logiqx//DTD ROM Management Datafile//EN\" \"http://www.logiqx.com/Dats/datafile.dtd\">\n";
        outFile << "<datafile>\n";
        outFile << "    <header>\n";
        outFile << "        <name>Trimmed ROMs</name>\n";
        outFile << "        <description>Automatically generated trimmed ROM datfile</description>\n";
        outFile << "        <version>" << getCurrentTimestamp() << "</version>\n";
        outFile << "        <author>RomTrimmer++</author>\n";
        outFile << "        <homepage>https://github.com/romtrimmer/romtrimmer-plusplus</homepage>\n";
        outFile << "    </header>\n\n";
        
        // Write each game entry
        for (const auto& originalEntry : originalEntries) {
            // Check if we have trimmed checksums for this game
            auto it = trimmedChecksums.find(originalEntry.name);
            if (it == trimmedChecksums.end()) {
                // No trimmed version, skip or include original?
                continue;
            }
            
            const auto& newChecksums = it->second;
            
            outFile << "    <game name=\"" << escapeXml(originalEntry.name) << "\">\n";
            outFile << "        <description>" << escapeXml(originalEntry.name) << "</description>\n";
            
            // Write ROM entry with new checksums
            outFile << "        <rom name=\"" << escapeXml(originalEntry.name) << "\" ";
            
            // Use new size if available, otherwise original
            auto sizeIt = newChecksums.find("size");
            if (sizeIt != newChecksums.end()) {
                outFile << "size=\"" << sizeIt->second << "\" ";
            } else if (!originalEntry.size.empty()) {
                outFile << "size=\"" << originalEntry.size << "\" ";
            }
            
            // Use new CRC32 if available
            auto crcIt = newChecksums.find("crc32");
            if (crcIt != newChecksums.end()) {
                outFile << "crc=\"" << crcIt->second << "\" ";
            } else if (!originalEntry.crc32.empty()) {
                outFile << "crc=\"" << originalEntry.crc32 << "\" ";
            }
            
            // Use new MD5 if available
            auto md5It = newChecksums.find("md5");
            if (md5It != newChecksums.end()) {
                outFile << "md5=\"" << md5It->second << "\" ";
            } else if (!originalEntry.md5.empty()) {
                outFile << "md5=\"" << originalEntry.md5 << "\" ";
            }
            
            // Use new SHA1 if available
            auto sha1It = newChecksums.find("sha1");
            if (sha1It != newChecksums.end()) {
                outFile << "sha1=\"" << sha1It->second << "\" ";
            } else if (!originalEntry.sha1.empty()) {
                outFile << "sha1=\"" << originalEntry.sha1 << "\" ";
            }
            
            outFile << "/>\n";
            outFile << "    </game>\n\n";
        }
        
        outFile << "</datafile>\n";
        outFile.close();
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

// ==================== BATCH PROCESSING ====================

std::unordered_map<std::string, RomEntry> DatIntegrator::verifyDirectoryAgainstDat(
    const std::string& directoryPath,
    const std::vector<RomEntry>& datEntries,
    bool recursive) {
    
    std::unordered_map<std::string, RomEntry> results;
    
    try {
        // Create lookup map for faster searching
        std::unordered_map<std::string, const RomEntry*> entryMap;
        for (const auto& entry : datEntries) {
            std::string filename = fs::path(entry.name).filename().string();
            entryMap[toLower(filename)] = &entry;
        }
        
        // Walk directory
        auto processFile = [&](const fs::path& filePath) {
            if (fs::is_regular_file(filePath)) {
                std::string filename = filePath.filename().string();
                std::string filenameLower = toLower(filename);
                
                auto it = entryMap.find(filenameLower);
                if (it != entryMap.end()) {
                    const RomEntry* expectedEntry = it->second;
                    bool verified = verifyRom(filePath.string(), *expectedEntry);
                    
                    RomEntry result = *expectedEntry;
                    result.status = verified ? "ok" : "modified";
                    results[filename] = result;
                } else {
                    // Not in DAT
                    RomEntry result;
                    result.name = filename;
                    result.status = "missing";
                    results[filename] = result;
                }
            }
        };
        
        if (recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
                processFile(entry.path());
            }
        } else {
            for (const auto& entry : fs::directory_iterator(directoryPath)) {
                processFile(entry.path());
            }
        }
        
    } catch (const std::exception& e) {
        // Log error
    }
    
    return results;
}

// ==================== PATCH DAT GENERATION ====================

bool DatIntegrator::generatePatchDat(
    const std::vector<RomEntry>& originalEntries,
    const std::string& patchDirectory,
    const std::string& outputDatPath) {
    
    try {
        std::ofstream outFile(outputDatPath);
        if (!outFile.is_open()) {
            return false;
        }
        
        // IPS DAT format header
        outFile << "[IPS]\n";
        outFile << "; Patch DAT generated by RomTrimmer++\n";
        outFile << "; " << getCurrentTimestamp() << "\n\n";
        
        // Write patch entries
        for (const auto& entry : originalEntries) {
            std::string patchFilename = fs::path(entry.name).stem().string() + ".ips";
            fs::path patchPath = fs::path(patchDirectory) / patchFilename;
            
            if (fs::exists(patchPath)) {
                // Get patch file size
                uintmax_t patchSize = fs::file_size(patchPath);
                
                // Write entry in IPS format
                outFile << entry.name << "=" << patchFilename << "\n";
                
                // Optional: Include checksums
                if (!entry.crc32.empty()) {
                    outFile << "; CRC32: " << entry.crc32 << "\n";
                }
                if (!entry.md5.empty()) {
                    outFile << "; MD5: " << entry.md5 << "\n";
                }
                outFile << "; Patch Size: " << patchSize << " bytes\n\n";
            }
        }
        
        outFile.close();
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

// ==================== DAT DIFFING ====================

std::vector<RomEntry> DatIntegrator::diffDatFiles(
    const std::vector<RomEntry>& dat1,
    const std::vector<RomEntry>& dat2) {
    
    std::vector<RomEntry> differences;
    
    // Create lookup maps
    std::unordered_map<std::string, const RomEntry*> map1;
    for (const auto& entry : dat1) {
        map1[toLower(entry.name)] = &entry;
    }
    
    std::unordered_map<std::string, const RomEntry*> map2;
    for (const auto& entry : dat2) {
        map2[toLower(entry.name)] = &entry;
    }
    
    // Find entries only in dat1
    for (const auto& [name, entry] : map1) {
        if (map2.find(name) == map2.end()) {
            RomEntry diffEntry = *entry;
            diffEntry.status = "only_in_first";
            differences.push_back(diffEntry);
        }
    }
    
    // Find entries only in dat2
    for (const auto& [name, entry] : map2) {
        if (map1.find(name) == map1.end()) {
            RomEntry diffEntry = *entry;
            diffEntry.status = "only_in_second";
            differences.push_back(diffEntry);
        }
    }
    
    // Find differing entries
    for (const auto& [name, entry1] : map1) {
        auto it = map2.find(name);
        if (it != map2.end()) {
            const RomEntry* entry2 = it->second;
            
            // Compare checksums
            bool different = false;
            std::string differencesStr;
            
            if (entry1->size != entry2->size) {
                different = true;
                differencesStr += "size ";
            }
            if (entry1->crc32 != entry2->crc32) {
                different = true;
                differencesStr += "crc32 ";
            }
            if (entry1->md5 != entry2->md5) {
                different = true;
                differencesStr += "md5 ";
            }
            if (entry1->sha1 != entry2->sha1) {
                different = true;
                differencesStr += "sha1 ";
            }
            
            if (different) {
                RomEntry diffEntry = *entry1;
                diffEntry.status = "different: " + differencesStr;
                differences.push_back(diffEntry);
            }
        }
    }
    
    return differences;
}

// ==================== ROM RENAMING ====================

bool DatIntegrator::renameFilesToDatNames(
    const std::string& directoryPath,
    const std::vector<RomEntry>& datEntries,
    bool dryRun) {
    
    try {
        // Create lookup map by checksum
        std::unordered_map<std::string, std::string> crcToName;
        std::unordered_map<std::string, std::string> md5ToName;
        std::unordered_map<std::string, std::string> sha1ToName;
        
        for (const auto& entry : datEntries) {
            if (!entry.crc32.empty()) {
                crcToName[toLower(entry.crc32)] = entry.name;
            }
            if (!entry.md5.empty()) {
                md5ToName[toLower(entry.md5)] = entry.name;
            }
            if (!entry.sha1.empty()) {
                sha1ToName[toLower(entry.sha1)] = entry.name;
            }
        }
        
        // Process files
        int renamed = 0;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.path())) {
                std::string filePath = entry.path().string();
                auto checksums = calculateChecksums(filePath);
                
                std::string newName;
                
                // Try to match by SHA1
                if (!checksums["sha1"].empty()) {
                    auto it = sha1ToName.find(toLower(checksums["sha1"]));
                    if (it != sha1ToName.end()) {
                        newName = it->second;
                    }
                }
                
                // Try to match by MD5
                if (newName.empty() && !checksums["md5"].empty()) {
                    auto it = md5ToName.find(toLower(checksums["md5"]));
                    if (it != md5ToName.end()) {
                        newName = it->second;
                    }
                }
                
                // Try to match by CRC32
                if (newName.empty() && !checksums["crc32"].empty()) {
                    auto it = crcToName.find(toLower(checksums["crc32"]));
                    if (it != crcToName.end()) {
                        newName = it->second;
                    }
                }
                
                // Rename if matched
                if (!newName.empty()) {
                    fs::path newPath = fs::path(directoryPath) / newName;
                    
                    if (!dryRun) {
                        try {
                            fs::rename(entry.path(), newPath);
                            renamed++;
                        } catch (const fs::filesystem_error& e) {
                            // Log error
                        }
                    } else {
                        // Just log what would happen
                        renamed++;
                    }
                }
            }
        }
        
        return renamed > 0;
        
    } catch (const std::exception& e) {
        return false;
    }
}

// ==================== HELPER FUNCTIONS ====================

std::string DatIntegrator::escapeXml(const std::string& input) {
    std::string output;
    output.reserve(input.length());
    
    for (char c : input) {
        switch (c) {
            case '&':  output.append("&amp;");  break;
            case '\"': output.append("&quot;"); break;
            case '\'': output.append("&apos;"); break;
            case '<':  output.append("&lt;");   break;
            case '>':  output.append("&gt;");   break;
            default:   output.push_back(c);     break;
        }
    }
    
    return output;
}

std::string DatIntegrator::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string DatIntegrator::toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

// ==================== EXTENDED DAT FORMATS ====================

bool DatIntegrator::exportToCsv(const std::vector<RomEntry>& entries,
                               const std::string& csvPath) {
    try {
        std::ofstream csvFile(csvPath);
        if (!csvFile.is_open()) {
            return false;
        }
        
        // Write header
        csvFile << "Filename,Size,CRC32,MD5,SHA1,Status\n";
        
        // Write entries
        for (const auto& entry : entries) {
            csvFile << "\"" << entry.name << "\","
                    << entry.size << ","
                    << entry.crc32 << ","
                    << entry.md5 << ","
                    << entry.sha1 << ","
                    << entry.status << "\n";
        }
        
        csvFile.close();
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

std::vector<RomEntry> DatIntegrator::importFromCsv(const std::string& csvPath) {
    std::vector<RomEntry> entries;
    
    try {
        std::ifstream csvFile(csvPath);
        if (!csvFile.is_open()) {
            return entries;
        }
        
        std::string line;
        bool firstLine = true;
        
        while (std::getline(csvFile, line)) {
            if (firstLine) {
                firstLine = false;
                continue; // Skip header
            }
            
            std::istringstream iss(line);
            std::string token;
            std::vector<std::string> tokens;
            
            // Parse CSV line
            bool inQuotes = false;
            std::string currentToken;
            
            for (char c : line) {
                if (c == '\"') {
                    inQuotes = !inQuotes;
                } else if (c == ',' && !inQuotes) {
                    tokens.push_back(currentToken);
                    currentToken.clear();
                } else {
                    currentToken += c;
                }
            }
            
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
            }
            
            if (tokens.size() >= 6) {
                RomEntry entry;
                entry.name = tokens[0];
                entry.size = tokens[1];
                entry.crc32 = tokens[2];
                entry.md5 = tokens[3];
                entry.sha1 = tokens[4];
                entry.status = tokens[5];
                entries.push_back(entry);
            }
        }
        
        csvFile.close();
        
    } catch (const std::exception& e) {
        // Return empty on error
    }
    
    return entries;
}

// ==================== ROM SET MANAGEMENT ====================

struct RomSetStats DatIntegrator::calculateRomSetStats(
    const std::vector<RomEntry>& entries) {
    
    RomSetStats stats;
    
    for (const auto& entry : entries) {
        stats.totalGames++;
        
        if (!entry.size.empty()) {
            try {
                size_t size = std::stoull(entry.size);
                stats.totalSize += size;
                
                if (size > stats.largestRom) {
                    stats.largestRom = size;
                    stats.largestRomName = entry.name;
                }
                
                if (stats.smallestRom == 0 || size < stats.smallestRom) {
                    stats.smallestRom = size;
                    stats.smallestRomName = entry.name;
                }
                
            } catch (...) {
                // Skip invalid size
            }
        }
        
        if (entry.status == "ok") {
            stats.verified++;
        } else if (entry.status == "missing") {
            stats.missing++;
        } else if (entry.status == "modified") {
            stats.modified++;
        }
    }
    
    stats.averageSize = stats.totalGames > 0 ? stats.totalSize / stats.totalGames : 0;
    
    return stats;
}

// ==================== DAT VALIDATION ====================

bool DatIntegrator::validateDatFile(const std::string& datPath,
                                   std::vector<std::string>& errors) {
    errors.clear();
    
    try {
        std::ifstream file(datPath);
        if (!file.is_open()) {
            errors.push_back("Cannot open file");
            return false;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        
        // Check for XML declaration
        if (content.find("<?xml") == std::string::npos) {
            // Not XML, might be ClrMamePro format
            if (content.find("clrmamepro") == std::string::npos &&
                content.find("rom (") == std::string::npos) {
                errors.push_back("Unrecognized DAT format");
                return false;
            }
        }
        
        // Basic XML validation (simplified)
        if (content.find("<datafile>") != std::string::npos) {
            // Logiqx XML format
            size_t openCount = countSubstring(content, "<game");
            size_t closeCount = countSubstring(content, "</game>");
            
            if (openCount != closeCount) {
                errors.push_back("Mismatched game tags");
                return false;
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        errors.push_back(std::string("Exception: ") + e.what());
        return false;
    }
}

size_t DatIntegrator::countSubstring(const std::string& str,
                                    const std::string& substr) {
    size_t count = 0;
    size_t pos = 0;
    
    while ((pos = str.find(substr, pos)) != std::string::npos) {
        count++;
        pos += substr.length();
    }
    
    return count;
}

// ==================== INTEGRATION WITH ROMTRIMMER ====================

std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
DatIntegrator::processDirectoryWithDat(
    const std::string& directoryPath,
    const std::string& datPath,
    bool trimFiles,
    const std::string& outputDir) {
    
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> results;
    
    try {
        // Parse DAT file
        auto datEntries = parseDatFile(datPath);
        
        // Verify each file against DAT
        auto verificationResults = verifyDirectoryAgainstDat(directoryPath, datEntries, false);
        
        // Process each file
        for (const auto& [filename, datEntry] : verificationResults) {
            if (datEntry.status == "ok" && trimFiles) {
                // File is verified, can be trimmed
                std::string inputPath = (fs::path(directoryPath) / filename).string();
                std::string outputPath = outputDir.empty() 
                    ? inputPath 
                    : (fs::path(outputDir) / filename).string();
                
                // Here you would call your trimming function
                // For now, just calculate new checksums
                auto checksums = calculateChecksums(inputPath);
                results[datEntry.name] = checksums;
            }
        }
        
    } catch (const std::exception& e) {
        // Log error
    }
    
    return results;
}
