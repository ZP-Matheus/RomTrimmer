#include "ConfigManager.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>

ConfigManager::ConfigManager() {
    // Tentar carregar configuração padrão
    loadConfig(getDefaultConfigPath());
}

void ConfigManager::load() {
    loadConfig(getDefaultConfigPath());
}

bool ConfigManager::loadConfig(const fs::path& configPath) {
    fs::path path = configPath;
    if (path.empty()) {
        path = getDefaultConfigPath();
    }
    
    if (!fs::exists(path)) {
        // Criar configuração padrão
        createDefaultConfig();
        return false;
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    configMap.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        parseLine(line);
    }
    
    currentConfigPath = path;
    return true;
}

bool ConfigManager::saveConfig(const fs::path& configPath) {
    fs::path path = configPath.empty() ? getDefaultConfigPath() : configPath;
    fs::create_directories(path.parent_path());

    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << "# RomTrimmer++ Configuration File\n";
    file << "# Generated automatically\n\n";

    auto writeSection = [&](const std::string& prefix, const std::string& sectionName){
        file << "[" << sectionName << "]\n";
        for (const auto& [key, value] : configMap) {
            if (key.find(prefix) == 0) {
                std::string cleanKey = key.substr(prefix.size());
                file << cleanKey << " = " << value << "\n";
            }
        }
        file << "\n";
    };

    writeSection("general.", "General");
    writeSection("trim.", "Trim Settings");
    writeSection("safety.", "Safety");
    writeSection("logging.", "Logging");

    return true;
}

// Em ConfigManager.cpp

void ConfigManager::createDefaultConfig() {
    auto def = [&](const std::string& k, const std::string& v) {
        if (configMap.find(k) == configMap.end())
            configMap[k] = v;
    };

    // General
    def("general.language", "unset");
    def("general.default_padding", "auto");
    def("general.create_backup", "true");
    def("general.recursive", "false");

    // Trim
    def("trim.safety_margin_kb", "64");
    def("trim.max_cut_percent", "60");
    def("trim.align_to", "4");

    // Safety
    def("safety.min_gba_size_mb", "1");
    def("safety.min_nds_size_mb", "8");
    def("safety.min_gb_size_kb", "32");
    def("safety.force_validation", "true");

    // Logging
    def("logging.level", "INFO");
    def("logging.to_file", "false");
    def("logging.max_files", "10");
    def("logging.max_size_mb", "10");
}


std::string ConfigManager::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        return it->second;
    }
    return defaultValue;
}

int ConfigManager::getInt(const std::string& key, int defaultValue) const {
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool ConfigManager::getBool(const std::string& key, bool defaultValue) const {
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        std::string value = it->second;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        return value == "true" || value == "1" || value == "yes" || value == "on";
    }
    return defaultValue;
}

double ConfigManager::getDouble(const std::string& key, double defaultValue) const {
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        try {
            return std::stod(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

void ConfigManager::setString(const std::string& key, const std::string& value) {
    configMap[key] = value;
}

void ConfigManager::setInt(const std::string& key, int value) {
    configMap[key] = std::to_string(value);
}

void ConfigManager::setBool(const std::string& key, bool value) {
    configMap[key] = value ? "true" : "false";
}

void ConfigManager::setDouble(const std::string& key, double value) {
    configMap[key] = std::to_string(value);
}

fs::path ConfigManager::getDefaultConfigPath() {
    // Unix-like: ~/.config/romtrimmer++/romtrimmer.conf
    // Windows: %APPDATA%/romtrimmer++/romtrimmer.conf
    
    fs::path configDir;
    
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        configDir = fs::path(appdata) / "romtrimmer++";
    }
#else
    const char* home = std::getenv("HOME");
    if (home) {
        configDir = fs::path(home) / ".config" / "romtrimmer++";
    }
#endif
    
    if (configDir.empty()) {
        configDir = fs::current_path();
    }
    
    return configDir / "romtrimmer.conf";
}

void ConfigManager::parseLine(const std::string& line) {
    std::string trimmed = trim(line);

    if (trimmed.empty() || isComment(trimmed))
        return;

    // Detectar seção
    if (trimmed.front() == '[' && trimmed.back() == ']') {
        currentSection = trim(trimmed.substr(1, trimmed.size() - 2));

        // normalizar nomes das seções
        std::transform(currentSection.begin(), currentSection.end(),
                       currentSection.begin(), ::tolower);

        if (currentSection == "general") currentSection = "general";
        else if (currentSection == "trim settings") currentSection = "trim";
        else if (currentSection == "safety") currentSection = "safety";
        else if (currentSection == "logging") currentSection = "logging";
        else currentSection.clear();

        return;
    }

    size_t eqPos = trimmed.find('=');
    if (eqPos == std::string::npos || currentSection.empty())
        return;

    std::string key = trim(trimmed.substr(0, eqPos));
    std::string value = trim(trimmed.substr(eqPos + 1));

    size_t commentPos = value.find('#');
    if (commentPos != std::string::npos)
        value = trim(value.substr(0, commentPos));

    if (!key.empty())
        configMap[currentSection + "." + key] = value;
}

std::string ConfigManager::trim(const std::string& str) const {
    const char* whitespace = " \t\r\n";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

bool ConfigManager::isComment(const std::string& line) const {
    return !line.empty() && (line[0] == '#' || line[0] == ';');
}
