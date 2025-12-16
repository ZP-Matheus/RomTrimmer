#pragma once
#include <string>
#include <map>
#include <algorithm>

enum class Language { EN, PT, ES, FR, AR, HI, BN, RU, ZH };

class LocalizationManager {
public:
    // Singleton: acesso global
    static LocalizationManager& instance();

    // Set language
    void setLanguage(Language lang);
    void setLanguage(const std::string& langCode);

    // Get translated string
    std::string getString(const std::string& key);
    std::string getLanguageCode() const;

    // Prompt user for language
    std::string promptForLanguage();

private:
    // Construtor privado
    LocalizationManager();

    // Carrega todas traduções
    void loadTranslations();

    Language currentLang;
    std::map<Language, std::map<std::string, std::string>> translations;
};

// Macro de tradução rápida
#define TR(key) LocalizationManager::instance().getString(key)