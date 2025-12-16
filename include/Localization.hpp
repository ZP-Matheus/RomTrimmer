#pragma once

/**
 * @file Localization.hpp
 * @brief Sistema de internacionalização do RomTrimmer++
 * 
 * Este arquivo fornece macros e funções para internacionalização
 * de todas as strings do programa.
 */

#include "LocalizationManager.hpp"
#include <string>
#include <stdexcept>
#include <vector>
#include <unordered_map>

// ==================== MACRO PRINCIPAL ====================
/**
 * @brief Macro para obter string traduzida
 * 
 * Exemplos de uso:
 * @code
 * std::cout << TR("WELCOME_MESSAGE") << std::endl;
 * logger->log(TR("ERROR_PROCESSING_FILE"), LogLevel::ERROR);
 * @endcode
 */
#define TR(key) LocalizationManager::instance().getString(key)

// ==================== VERIFICAÇÃO EM TEMPO DE COMPILAÇÃO ====================
#ifdef _DEBUG
    /**
     * @brief Verifica se uma chave de tradução existe (apenas em debug)
     */
    inline void TR_CHECK(const std::string& key) {
        // Tentar obter a tradução, se falhar será óbvio
        LocalizationManager::instance().getString(key);
    }
#else
    #define TR_CHECK(key) ((void)0)
#endif

// ==================== FUNÇÕES AUXILIARES ====================
namespace Localization {
    
    /**
     * @brief Obtém a língua atual do sistema
     * @return Código da língua (ex: "en", "pt", "es")
     */
    inline std::string getCurrentLanguage() {
        return LocalizationManager::instance().getLanguageCode();
    }
    
    /**
     * @brief Define a língua do programa
     * @param langCode Código da língua (ex: "en", "pt", "es")
     * @throws std::invalid_argument Se o código da língua não for suportado
     */
    inline void setLanguage(const std::string& langCode) {
        LocalizationManager::instance().setLanguage(langCode);
    }
    
    /**
     * @brief Obtém todas as línguas suportadas
     * @return Vetor com códigos das línguas suportadas
     */
    inline std::vector<std::string> getSupportedLanguages() {
        return {
            "en",  // Inglês
            "pt",  // Português
            "es",  // Espanhol
            "fr",  // Francês
            "ar",  // Árabe
            "hi",  // Hindi
            "bn",  // Bengali
            "ru",  // Russo
            "zh"   // Chinês
        };
    }
    
    /**
     * @brief Obtém o nome completo da língua
     * @param langCode Código da língua
     * @return Nome completo da língua
     */
    inline std::string getLanguageName(const std::string& langCode) {
        static const std::unordered_map<std::string, std::string> languageNames = {
            {"en", "English"},
            {"pt", "Português"},
            {"es", "Español"},
            {"fr", "Français"},
            {"ar", "العربية"},
            {"hi", "हिन्दी"},
            {"bn", "বাংলা"},
            {"ru", "Русский"},
            {"zh", "中文"}
        };
        
        auto it = languageNames.find(langCode);
        return it != languageNames.end() ? it->second : "Unknown";
    }
    
    /**
     * @brief Formata bytes em formato legível
     * @param bytes Número de bytes
     * @return String formatada (ex: "1.23 MB")
     */
    inline std::string formatBytes(size_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double size = static_cast<double>(bytes);
        
        while (size >= 1024.0 && unitIndex < 4) {
            size /= 1024.0;
            unitIndex++;
        }
        
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[unitIndex]);
        return std::string(buffer);
    }
    
    /**
     * @brief Formata porcentagem
     * @param value Valor entre 0.0 e 1.0
     * @param decimals Número de casas decimais
     * @return String formatada (ex: "45.67%")
     */
    inline std::string formatPercent(double value, int decimals = 2) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.*f%%", decimals, value * 100);
        return std::string(buffer);
    }
    
    /**
     * @brief Obtém direção do texto da língua atual
     * @return "ltr" (left-to-right) ou "rtl" (right-to-left)
     */
    inline std::string getTextDirection() {
        std::string lang = getCurrentLanguage();
        return (lang == "ar" || lang == "he") ? "rtl" : "ltr";
    }
}

// ==================== EXCEÇÕES DE LOCALIZAÇÃO ====================
class LocalizationException : public std::runtime_error {
public:
    explicit LocalizationException(const std::string& message)
        : std::runtime_error("Localization error: " + message) {}
};

class MissingTranslationException : public LocalizationException {
public:
    explicit MissingTranslationException(const std::string& key)
        : LocalizationException("Missing translation for key: " + key) {}
};