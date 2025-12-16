#pragma once
#include <string>
#include <vector>

struct ValidationResult {
    bool isValid = false;
    std::string message;
    std::vector<std::string> warnings;
    std::vector<std::string> suggestions;
};