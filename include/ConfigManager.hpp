#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;

class ConfigManager {
public:
    void load();
    ConfigManager();
    ~ConfigManager() = default;
    
    void createDefaultConfig();
    
    bool loadConfig(const fs::path& configPath = "");
    bool saveConfig(const fs::path& configPath = "");
    
    std::string currentSection;
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;
    double getDouble(const std::string& key, double defaultValue = 0.0) const;
    
    void setString(const std::string& key, const std::string& value);
    void setInt(const std::string& key, int value);
    void setBool(const std::string& key, bool value);
    void setDouble(const std::string& key, double value);
    
    static fs::path getDefaultConfigPath();
    
private:

    std::unordered_map<std::string, std::string> configMap;
    fs::path currentConfigPath;
    
    void parseLine(const std::string& line);
    std::string trim(const std::string& str) const;
    bool isComment(const std::string& line) const;
};