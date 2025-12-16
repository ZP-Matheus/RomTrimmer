#pragma once
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>

class RomInfo {
public:
    std::string filename;
    std::string data;
    size_t originalSize;
    size_t trimmedSize;
    bool wasTrimmed;
    
    // Metadados
    std::string title;
    std::string gameCode;
    std::string region;
    uint32_t crc32;
    std::string md5;
};

class Plugin {
public:
    virtual ~Plugin() = default;
    
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    
    // Hooks de processamento
    virtual bool preProcess(RomInfo& romInfo) { return true; }
    virtual bool postProcess(RomInfo& romInfo) { return true; }
    virtual bool validateTrim(RomInfo& romInfo, size_t trimPoint) { return true; }
    
    // Análise específica
    virtual std::string analyze(const RomInfo& romInfo) { return ""; }
};

class PluginManager {
public:
    PluginManager();
    ~PluginManager();
    
    bool loadPlugin(const std::string& path);
    bool loadAllPlugins(const std::string& directory);
    
    void registerPlugin(std::unique_ptr<Plugin> plugin);
    
    // Executar hooks
    bool runPreProcess(RomInfo& romInfo);
    bool runPostProcess(RomInfo& romInfo);
    bool runValidateTrim(RomInfo& romInfo, size_t trimPoint);
    
    std::vector<std::string> analyzeAll(const RomInfo& romInfo);
    
private:
    std::vector<std::unique_ptr<Plugin>> plugins;
};