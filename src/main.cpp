/**
 * @file main.cpp
 * @brief Ponto de entrada principal do RomTrimmer++
 */

#include "Localization.hpp"
#include "RomTrimmer.hpp"
#include "ConfigManager.hpp"
#include "Logger.hpp"
#include "Version.hpp"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <signal.h>
#include <algorithm>

// ==================== VARIÁVEIS GLOBAIS ====================
std::unique_ptr<RomTrimmer> g_romTrimmer;
std::unique_ptr<Logger> g_emergencyLogger;

// ==================== DETECTAR LÍNGUA DO SISTEMA ====================
std::string detectSystemLanguage() {
    const char* langEnv = std::getenv("LANG");
    if (langEnv != nullptr) {
        std::string lang(langEnv);
        size_t underscore = lang.find('_');
        if (underscore != std::string::npos) return lang.substr(0, underscore);
        size_t dot = lang.find('.');
        if (dot != std::string::npos) return lang.substr(0, dot);
        return lang;
    }
    const char* lcAll = std::getenv("LC_ALL");
    if (lcAll != nullptr) return std::string(lcAll).substr(0, 2);
    const char* lcMessages = std::getenv("LC_MESSAGES");
    if (lcMessages != nullptr) return std::string(lcMessages).substr(0, 2);
    return "en";
}

// ==================== PRIMEIRA CONFIGURAÇÃO DE LÍNGUA ====================
bool promptFirstTimeLanguage(ConfigManager& config) {
    std::cout << "\n🌍 Select your language / Selecione seu idioma:\n\n";
    std::cout << "1. English\n2. Português (Brasil)\n3. Español\n4. Français\n";
    std::cout << "5. العربية\n6. हिन्दी\n7. বাংলা\n8. Русский\n9. 中文\n";
    std::cout << "\nEnter choice (1-9): ";

    int choice;
    if (!(std::cin >> choice)) return false;

    std::string langCode;
    switch (choice) {
        case 1: langCode = "en"; break;
        case 2: langCode = "pt"; break;
        case 3: langCode = "es"; break;
        case 4: langCode = "fr"; break;
        case 5: langCode = "ar"; break;
        case 6: langCode = "hi"; break;
        case 7: langCode = "bn"; break;
        case 8: langCode = "ru"; break;
        case 9: langCode = "zh"; break;
        default: return false;
    }

    config.setString("general.language", langCode);
    config.saveConfig();
    std::cout << "\n✅ Language set to " << langCode << std::endl;
    return true;
}

// ==================== CONFIGURAÇÃO DE LOCALIZAÇÃO ====================
std::string setupLocalization(ConfigManager& config, int argc, char* argv[]) {
    std::string langCode;
    bool langOverridden = false;

    // Argumentos de linha de comando
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--lang") == 0 || std::strcmp(argv[i], "-l") == 0) {
            if (i + 1 < argc) {
                langCode = argv[i + 1];
                langOverridden = true;
                break;
            }
        }
    }

    if (langCode.empty()) {
        langCode = config.getString("general.language", "unset");
    }

    if (langCode == "unset") {
        if (!promptFirstTimeLanguage(config)) langCode = "en";
        else langCode = config.getString("general.language", "en");
    } else if (langCode == "auto") {
        langCode = detectSystemLanguage();
        auto supported = Localization::getSupportedLanguages();
        if (std::find(supported.begin(), supported.end(), langCode) == supported.end())
            langCode = "en";
    }

    try {
        LocalizationManager::instance().setLanguage(langCode);
        std::cout << TR("LANGUAGE_SET") << ": " 
                  << Localization::getLanguageName(langCode) 
                  << " (" << langCode << ")" << std::endl;
    } catch (...) {
        LocalizationManager::instance().setLanguage("en");
        langCode = "en";
    }

    return langCode;
}

// ==================== FUNÇÃO PRINCIPAL ====================
int main(int argc, char* argv[]) {
    g_emergencyLogger = std::make_unique<Logger>();

    try {
        // Banner inicial
        std::cout << "\n"
                  << "╔══════════════════════════════════════════╗\n"
                  << "║          RomTrimmer++ v" << ROMTRIMMER_VERSION_STRING << "             ║\n"
                  << "║    " << TR("A_POWERFUL_ROM_TRIMMING_UTILITY") << "       ║\n"
                  << "╚══════════════════════════════════════════╝\n";

        // ConfigManager
        ConfigManager config;
config.loadConfig(config.getDefaultConfigPath());

std::string langInConfig = config.getString("general.language", "unset");
if (langInConfig == "unset") {
    promptFirstTimeLanguage(config);
    config.saveConfig();
}

        // Localização
        std::string langCode = setupLocalization(config, argc, argv);

        // Verificar argumentos mínimos
        if (argc < 2) {
                std::cout << TR("USAGE") << "\n";
    std::cout << "  romtrimmer++ [OPTIONS] -i <arquivo>\n";
    std::cout << "  romtrimmer++ [OPTIONS] -p <diretório>\n";
    std::cout << "  romtrimmer++ -i roms.zip --compressed\n";
    std::cout << "  romtrimmer++ -p downloads/ -e \"nds,gba,nes\"\n\n";
    
    std::cout << TR("EXAMPLES") << "\n";
    std::cout << "  " << TR("EXAMPLE_TRIM_SINGLE") << "\n";
    std::cout << "  " << TR("EXAMPLE_PROCESS_DIR") << "\n";
    std::cout << "  " << TR("EXAMPLE_ANALYZE_ONLY") << "\n";
    std::cout << "  Processar ZIP: romtrimmer++ -i roms.zip --compressed\n";
    std::cout << "  Extensões personalizadas: romtrimmer++ -p roms/ -e \"iso,bin,img\"\n\n"
      << "  romtrimmer++ --help\n";
            return EXIT_SUCCESS;
        }

        // Executar RomTrimmer
        g_romTrimmer = std::make_unique<RomTrimmer>();
        g_romTrimmer->run(argc, argv);

        // Salvar configuração ao final
        config.saveConfig();

        return EXIT_SUCCESS;

    } catch (const std::exception& e) {
        std::cerr << "\n❌ " << TR("CRITICAL_ERROR") << ": " << e.what() << std::endl;
        if (g_emergencyLogger)
            g_emergencyLogger->log("Exceção não tratada: " + std::string(e.what()), LogLevel::ERROR);
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "\n❌ " << TR("UNKNOWN_ERROR") << std::endl;
        if (g_emergencyLogger)
            g_emergencyLogger->log("Erro desconhecido não tratado", LogLevel::ERROR);
        return EXIT_FAILURE;
    }
}

// ==================== DEFINIÇÕES DE VERSÃO ====================
#ifndef VERSION_STRING
#define VERSION_STRING "1.0.0"
#endif
#ifndef BUILD_DATE
#define BUILD_DATE __DATE__
#endif
#ifndef BUILD_TIME
#define BUILD_TIME __TIME__
#endif

std::string getVersionInfo() {
    std::stringstream ss;
    ss << "RomTrimmer++ v" << VERSION_STRING << "\n"
       << "Build: " << BUILD_DATE << " " << BUILD_TIME << "\n"
       << "Supported ROMs: GBA, NDS, GB, GBC, NES, SNES, N64\n"
       << "License: MIT\n"
       << "Repository: https://github.com/romtrimmer/romtrimmer-plusplus\n";
    return ss.str();
}
